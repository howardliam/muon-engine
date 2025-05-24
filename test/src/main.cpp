#include <memory>
#include <optional>
#include <print>
#include <fstream>
#include <thread>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RIGHT_HANDED
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/geometric.hpp>

#include <muon/engine/renderer/descriptor/pool.hpp>
#include <muon/engine/renderer/descriptor/writer.hpp>
#include <muon/engine/renderer/descriptor/set_layout.hpp>
#include <muon/engine/renderer/frame_graph/frame_graph.hpp>
#include <muon/engine/renderer/frame_graph/blackboard.hpp>
#include <muon/engine/renderer/pipeline/compute.hpp>
#include <muon/engine/renderer/pipeline/graphics.hpp>
#include <muon/engine/renderer/pipeline/layout.hpp>
#include <muon/engine/renderer/buffer.hpp>
#include <muon/engine/renderer/debug_ui.hpp>
#include <muon/engine/renderer/device.hpp>
#include <muon/engine/renderer/frame_handler.hpp>
#include <muon/engine/renderer/image.hpp>
#include <muon/engine/renderer/mesh.hpp>
#include <muon/engine/renderer/shader.hpp>
#include <muon/engine/renderer/swapchain.hpp>
#include <muon/engine/renderer/texture.hpp>
#include <muon/engine/platform/window.hpp>
#include <muon/engine/utils/color.hpp>
#include <muon/engine/core/assert.hpp>
#include <muon/engine/core/log.hpp>

#include <muon/common/log/logger.hpp>
#include <muon/asset/image.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>

#include <vk_mem_alloc_enums.hpp>
#include <vulkan/vulkan.hpp>

#include <imgui.h>

#include "g_buffer.hpp"

void startMessage() {
    #ifdef MU_DEBUG_ENABLED
    MU_INFO("running in debug");
    auto vkMajor = VK_API_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE);
    auto vkMinor = VK_API_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE);
    auto vkPatch = VK_API_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE);
    MU_INFO("Vulkan header version: {}.{}.{}", vkMajor, vkMinor, vkPatch);
    #else
    MU_INFO("running in release");
    #endif
}

int main() {
    mu::Log::init();

    startMessage();

    mu::ShaderCompiler shaderCompiler;
    shaderCompiler.addShaders("./test/assets/shaders");
    shaderCompiler.compile();

    mu::Window window = mu::Window::Builder()
        .setDimensions(1600, 900)
        .setInitialDisplayMode(mu::Window::DisplayMode::Windowed)
        .setTitle("Testing")
        .build();

    auto windowIcon = mu::asset::decodePng("./muon-logo.png");
    window.setIcon(windowIcon->data, windowIcon->width, windowIcon->height, windowIcon->channels);

    bool mouseGrab{false};

    mu::Device device(window);
    mu::FrameHandler frameHandler(window, device);
    mu::DebugUi debugUi(window, device);

    GBufferPass gBufferPass(device);
    gBufferPass.createResources(window.getExtent());

    struct Ubo {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 transform;
    };

    auto uboBuffer = std::make_unique<mu::Buffer>(
        device,
        sizeof(Ubo),
        1,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vma::MemoryUsage::eCpuCopy
    );

    auto bufferInfo = uboBuffer->getDescriptorInfo();

    mu::DescriptorWriter(*gBufferPass.getGlobalPool(), *gBufferPass.getGlobalSetLayout())
        .addBufferWrite(1, 0, &bufferInfo)
        .writeAll(gBufferPass.getGlobalSet());

    auto usageFlags = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
    auto accessFlags = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite;

    std::unique_ptr computeImagePool = mu::DescriptorPool::Builder(device)
        .addPoolSize(vk::DescriptorType::eStorageImage, 4)
        .buildUniquePtr();

    std::unique_ptr compositeSetLayout = mu::DescriptorSetLayout::Builder(device)
        .addBinding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1)
        .buildUniquePtr();

    auto compositeSet = compositeSetLayout->createSet(*computeImagePool);

    std::unique_ptr computeImageB = mu::Image::Builder(device)
        .setExtent(window.getExtent())
        .setFormat(vk::Format::eR8G8B8A8Unorm)
        .setImageUsageFlags(usageFlags)
        .setImageLayout(vk::ImageLayout::eGeneral)
        .setAccessFlags(accessFlags)
        .setPipelineStageFlags(vk::PipelineStageFlagBits2::eComputeShader)
        .buildUniquePtr();

    std::unique_ptr computeSetLayout = mu::DescriptorSetLayout::Builder(device)
        .addBinding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 2)
        .buildUniquePtr();

    auto computeSet = computeSetLayout->createSet(*computeImagePool);
    mu::DescriptorWriter(*computeImagePool, *computeSetLayout)
        .addImageWrite(0, 1, computeImageB->getDescriptorInfo())
        .writeAll(computeSet);

    auto uiComposite = mu::ComputePipeline::Builder(device)
        .setShader("./test/assets/shaders/uicomposite.comp.spv")
        .setDescriptorSetLayouts({ computeSetLayout->getSetLayout(), compositeSetLayout->getSetLayout() })
        .buildUniquePtr();

    auto toneMap = mu::ComputePipeline::Builder(device)
        .setShader("./test/assets/shaders/tonemap.comp.spv")
        .setDescriptorSetLayouts({ computeSetLayout->getSetLayout() })
        .buildUniquePtr();

    auto swizzle = mu::ComputePipeline::Builder(device)
        .setShader("./test/assets/shaders/swizzle.comp.spv")
        .setDescriptorSetLayouts({ computeSetLayout->getSetLayout() })
        .buildUniquePtr();

    bool screenshotRequested{false};

    auto size = window.getExtent().width * window.getExtent().height;

    std::unique_ptr stagingBuffer = std::make_unique<mu::Buffer>(
        device,
        4,
        size,
        vk::BufferUsageFlagBits::eTransferDst,
        vma::MemoryUsage::eGpuToCpu
    );

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoords;
    };

    std::vector<Vertex> vertices = {
        {{-0.5, -0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0}},
        {{0.5, -0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0}},
        {{0.5, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0}},
        {{-0.5, 0.5, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0}},
    };

    std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
    auto vertexData = mu::Mesh::getRawVertexData(vertices);

    mu::Mesh square(device, vertexData, 32, indices);

    frameHandler.beginFrameTiming();

    glm::mat4 transform = glm::mat4{1.0f};
    transform = glm::scale(transform, glm::vec3{2.0f});

    float seconds{0.0};
    uint32_t frames{0};
    uint32_t frameRate{0};

    glm::vec3 position{0.0, 0.0, 10.0};
    glm::vec3 up{0.0, 1.0, 0.0};
    glm::vec3 orientation{0.0, 0.0, -1.0};

    float sensitivity{0.1};

    float maxSpeed{1.2}; // m s^-1
    float acceleration{0.3};
    float friction{5.0};
    glm::vec3 velocity{0.0, 0.0, 0.0};
    glm::vec3 moveDir{0.0};

    float rotationSpeed{15.0};
    bool xAxis{false};
    bool yAxis{false};
    bool zAxis{false};

    auto handleInput = [&](SDL_Event &event, float dt) -> void {
        glm::vec3 forward = glm::normalize(orientation);
        glm::vec3 right = glm::normalize(glm::cross(forward, up));

        glm::vec3 inputDir{0.0};
        if (event.type == SDL_EVENT_KEY_DOWN) {
            if (event.key.scancode == SDL_SCANCODE_W) {
                inputDir += forward;
            }
            if (event.key.scancode == SDL_SCANCODE_S) {
                inputDir -= forward;
            }
            if (event.key.scancode == SDL_SCANCODE_A) {
                inputDir -= right;
            }
            if (event.key.scancode == SDL_SCANCODE_D) {
                inputDir += right;
            }
        }

        moveDir = glm::vec3{0.0};
        if (glm::length(inputDir) > 0.0f) {
            moveDir = glm::normalize(inputDir);
        }

        if (glm::length(moveDir) < 0.01f) {
            return;
        } else {
            glm::vec3 accelerationDir = moveDir * acceleration;
            velocity += accelerationDir * dt;

            if (glm::length(velocity) > maxSpeed) {
                velocity = glm::normalize(velocity) * maxSpeed;
            }
        }

    };

    while (window.isOpen()) {
        float dt = frameHandler.getFrameTime();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            debugUi.pollEvents(&event);
            handleInput(event, dt);

            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                if (mouseGrab) {
                    float dt = frameHandler.getFrameTime();
                    float yaw = -event.motion.xrel * sensitivity * dt;
                    float pitch = event.motion.yrel * sensitivity * dt;

                    pitch = glm::clamp(pitch, -1.54f, 1.54f);

                    glm::quat quatYaw = glm::angleAxis(yaw, up);

                    glm::vec3 right = glm::normalize(glm::cross(orientation, up));
                    glm::quat quatPitch = glm::angleAxis(pitch, right);

                    glm::quat rotation = quatYaw * quatPitch;

                    orientation = rotation * orientation;
                }
            }

            if (event.type == SDL_EVENT_QUIT) {
                window.setToClose();
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    mouseGrab = !mouseGrab;

                    SDL_SetWindowMouseGrab(window.getWindow(), mouseGrab);
                    SDL_SetWindowRelativeMouseMode(window.getWindow(), mouseGrab);
                }

                if (event.key.scancode == SDL_SCANCODE_F1) {
                    orientation = {0.0, 0.0, -1.0};
                }

                if (event.key.scancode == SDL_SCANCODE_F2) {
                    screenshotRequested = true;
                    MU_INFO("screenshot requested");
                }
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                window.resize(event.window.data1, event.window.data2);
            }
        }

        if (glm::length(moveDir) < 0.01f) {
            velocity -= velocity * glm::min(dt * friction, 1.0f);
        }

        position += velocity;

        if (seconds >= 1.0) {
            frameRate = frames;
            seconds = 0;
            frames = 0;
        }

        if (window.wasResized()) {
            device.getDevice().waitIdle();

            auto extent = window.getExtent();

            debugUi.recreateSizedResources();

            gBufferPass.createResources(extent);

            computeImageB = mu::Image::Builder(device)
                .setExtent(extent)
                .setFormat(vk::Format::eR8G8B8A8Unorm)
                .setImageUsageFlags(usageFlags)
                .setImageLayout(vk::ImageLayout::eGeneral)
                .setAccessFlags(accessFlags)
                .setPipelineStageFlags(vk::PipelineStageFlagBits2::eComputeShader)
                .buildUniquePtr();

            mu::DescriptorWriter(*computeImagePool, *computeSetLayout)
                .addImageWrite(0, 1, computeImageB->getDescriptorInfo())
                .writeAll(computeSet);

            size = extent.width * extent.height;
            stagingBuffer = std::make_unique<mu::Buffer>(
                device,
                4,
                size,
                vk::BufferUsageFlagBits::eTransferDst,
                vma::MemoryUsage::eGpuToCpu
            );

            frameHandler.recreateSwapchain(extent);

            window.resetResized();

            continue;
        }

        if (xAxis || yAxis || zAxis) {
            glm::vec3 rotationAxes{0.0};
            if (xAxis) rotationAxes.x = 1.0;
            if (yAxis) rotationAxes.y = 1.0;
            if (zAxis) rotationAxes.z = 1.0;

            transform = glm::rotate(transform, glm::radians(rotationSpeed) * frameHandler.getFrameTime(), rotationAxes);
        }

        Ubo ubo{};
        ubo.view = glm::lookAt(position, position + orientation, up);
        ubo.projection = glm::perspective(glm::radians(45.0f), frameHandler.getAspectRatio(), 0.1f, 1000.0f);
        ubo.transform = transform;

        auto _ = uboBuffer->map();
        uboBuffer->writeToBuffer(&ubo);
        uboBuffer->flush();
        uboBuffer->unmap();

        const auto cmd = frameHandler.beginFrame();

        gBufferPass.getAlbedoImage()->transitionLayout(cmd,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput
        );
        gBufferPass.drawFrame(cmd, window.getExtent(), square);

        gBufferPass.getAlbedoImage()->transitionLayout(cmd,
            vk::ImageLayout::eGeneral,
            vk::AccessFlagBits2::eShaderRead,
            vk::PipelineStageFlagBits2::eComputeShader
        );
        mu::DescriptorWriter(*computeImagePool, *computeSetLayout)
            .addImageWrite(0, 0, gBufferPass.getAlbedoImage()->getDescriptorInfo())
            .writeAll(computeSet);

        debugUi.getImage()->transitionLayout(cmd,
            vk::ImageLayout::eColorAttachmentOptimal,
            vk::AccessFlagBits2::eColorAttachmentWrite,
            vk::PipelineStageFlagBits2::eColorAttachmentOutput
        );

        debugUi.beginRendering(cmd);

        {
            ImGui::Begin("Debug UI");

            ImGui::Text("FPS: %d", frameRate);

            ImGui::DragFloat("Sensitivity", &sensitivity, 0.1, 0.0, 1.0);
            float oriArray[3] = {orientation.x, orientation.y, orientation.z};
            ImGui::DragFloat3("Orientation", oriArray, 0.01, -1.0, 1.0);
            orientation.x = oriArray[0];
            orientation.y = oriArray[1];
            orientation.z = oriArray[2];

            ImGui::DragFloat("Max speed", &maxSpeed, 0.1, 0.0, 10.0);
            ImGui::DragFloat("Acceleration", &acceleration, 0.1, 0.0, 10.0);

            ImGui::Text("Current speed: %f", glm::length(velocity));
            ImGui::Text("Position: %f, %f, %f", position.x, position.y, position.z);
            ImGui::Text("Move vector: %f, %f, %f", moveDir.x, moveDir.y, moveDir.z);

            ImGui::End();
        }

        {
            ImGui::Begin("Model control");

            ImGui::DragFloat("Spin speed", &rotationSpeed, 1.0, 0.0, 720.0);
            ImGui::Checkbox("x-axis", &xAxis);
            ImGui::Checkbox("y-axis", &yAxis);
            ImGui::Checkbox("z-axis", &zAxis);

            ImGui::End();
        }

        debugUi.endRendering(cmd);

        debugUi.getImage()->transitionLayout(cmd,
            vk::ImageLayout::eGeneral,
            vk::AccessFlagBits2::eShaderRead,
            vk::PipelineStageFlagBits2::eComputeShader
        );

        mu::DescriptorWriter(*computeImagePool, *compositeSetLayout)
            .addImageWrite(0, 0, debugUi.getImage()->getDescriptorInfo())
            .writeAll(compositeSet);

        uiComposite->bind(cmd, { computeSet, compositeSet });
        uiComposite->dispatch(cmd, window.getExtent(), {32, 32, 1});

        toneMap->bind(cmd, { computeSet });
        toneMap->dispatch(cmd, window.getExtent(), {32, 32, 1});

        if (screenshotRequested) {
            gBufferPass.getAlbedoImage()->transitionLayout(cmd,
                vk::ImageLayout::eTransferSrcOptimal,
                vk::AccessFlagBits2::eTransferRead,
                vk::PipelineStageFlagBits2::eTransfer
            );

            auto extent = window.getExtent();

            vk::BufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = vk::Offset3D{0, 0, 0};
            region.imageExtent = vk::Extent3D{extent, 1};

            cmd.copyImageToBuffer(
                gBufferPass.getAlbedoImage()->getImage(),
                vk::ImageLayout::eTransferSrcOptimal,
                stagingBuffer->getBuffer(),
                1,
                &region
            );

            gBufferPass.getAlbedoImage()->transitionLayout(cmd,
                vk::ImageLayout::eColorAttachmentOptimal,
                vk::AccessFlagBits2::eColorAttachmentWrite,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput
            );
        }

        swizzle->bind(cmd, { computeSet });
        swizzle->dispatch(cmd, window.getExtent(), {32, 32, 1});

        computeImageB->transitionLayout(cmd,
            vk::ImageLayout::eTransferSrcOptimal,
            vk::AccessFlagBits2::eTransferRead,
            vk::PipelineStageFlagBits2::eTransfer
        );

        frameHandler.copyImageToSwapchain(computeImageB->getImage());

        computeImageB->transitionLayout(cmd,
            vk::ImageLayout::eGeneral,
            accessFlags,
            vk::PipelineStageFlagBits2::eComputeShader
        );

        frameHandler.endFrame();

        if (screenshotRequested) {
            if (stagingBuffer->map() != vk::Result::eSuccess) {
                continue;
            }

            auto extent = window.getExtent();

            mu::asset::Image image{};
            image.width = extent.width;
            image.height = extent.height;
            image.channels = 4;
            image.bitDepth = 8;
            image.data.resize(stagingBuffer->getBufferSize());

            std::memcpy(image.data.data(), stagingBuffer->getMappedMemory(), stagingBuffer->getBufferSize());

            stagingBuffer->unmap();

            std::thread([image]() {
                auto png = mu::asset::encodePng(image);

                std::ofstream outputFile("./screenshot.png");
                outputFile.write(reinterpret_cast<char *>(png->data()), png->size());

                MU_INFO("screenshot saved");
            }).detach();

            screenshotRequested = false;
        }

        frameHandler.updateFrameTiming();

        seconds += frameHandler.getFrameTime();
        frames += 1;
    }

    device.getDevice().waitIdle();
}
