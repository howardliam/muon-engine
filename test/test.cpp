#include <limits>
#include <memory>
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

#include <muon/engine/fg/framegraph.hpp>
#include <muon/engine/fg/blackboard.hpp>
#include <muon/engine/debugui.hpp>
#include <muon/engine/descriptor/writer.hpp>
#include <muon/engine/shader.hpp>
#include <muon/utils/color.hpp>
#include <muon/engine/pipeline/compute.hpp>
#include <muon/engine/pipeline/graphics.hpp>
#include <muon/engine/buffer.hpp>
#include <muon/engine/descriptor/pool.hpp>
#include <muon/engine/descriptor/setlayout.hpp>
#include <muon/engine/device.hpp>
#include <muon/engine/framehandler.hpp>
#include <muon/engine/image.hpp>
#include <muon/engine/model.hpp>
#include <muon/engine/swapchain.hpp>
#include <muon/engine/window.hpp>
#include <muon/log/logger.hpp>
#include <muon/asset/image.hpp>
#include <muon/engine/system/graphics.hpp>
#include <muon/engine/texture.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>

#include <spdlog/spdlog.h>

#include <vk_mem_alloc_enums.hpp>
#include <vulkan/vulkan.hpp>

#include <imgui.h>

using namespace muon;

class Logger : public log::ILogger {
public:
    Logger() : log::ILogger() {
        spdlog::set_level(spdlog::level::trace);
    }

private:
    void traceImpl(const std::string &message) override {
        spdlog::trace(message);
    }

    void debugImpl(const std::string &message) override {
        spdlog::debug(message);
    }

    void infoImpl(const std::string &message) override {
        spdlog::info(message);
    }

    void warnImpl(const std::string &message) override {
        spdlog::warn(message);
    }

    void errorImpl(const std::string &message) override {
        spdlog::error(message);
    }
};

class RenderSystemTest : public engine::GraphicsSystem {
public:
    RenderSystemTest(
        engine::Device &device,
        const std::vector<vk::DescriptorSetLayout> &setLayouts,
        const std::vector<vk::PushConstantRange> &pushConstants
    ) : engine::GraphicsSystem(device, setLayouts, pushConstants)  {

    }

    void renderModel(vk::CommandBuffer commandBuffer, vk::DescriptorSet set, const engine::Model &model) {
        pipeline->bind(commandBuffer);

        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            pipelineLayout,
            0,
            1,
            &set,
            0,
            nullptr
        );

        model.bind(commandBuffer);
        model.draw(commandBuffer);
    }

protected:
    void createPipeline(const vk::PipelineRenderingCreateInfo &renderingInfo) override {
        engine::GraphicsPipeline::ConfigInfo configInfo;
        engine::GraphicsPipeline::defaultConfigInfo(configInfo);

        configInfo.renderingInfo = renderingInfo;
        configInfo.pipelineLayout = pipelineLayout;

        pipeline = engine::GraphicsPipeline::Builder(device)
            .addShader(vk::ShaderStageFlagBits::eVertex, "./test/assets/shaders/shader.vert.spv")
            .addShader(vk::ShaderStageFlagBits::eFragment, "./test/assets/shaders/shader.frag.spv")
            .buildUniquePtr(configInfo);
    }
};

int main() {
    auto logger = std::make_shared<Logger>();
    log::setLogger(logger.get());

    #ifndef NDEBUG
    logger->info("running in debug");
    auto vkMajor = VK_API_VERSION_MAJOR(VK_HEADER_VERSION_COMPLETE);
    auto vkMinor = VK_API_VERSION_MINOR(VK_HEADER_VERSION_COMPLETE);
    auto vkPatch = VK_API_VERSION_PATCH(VK_HEADER_VERSION_COMPLETE);
    logger->info("Vulkan header version: {}.{}.{}", vkMajor, vkMinor, vkPatch);
    #else
    logger->info("running in release");
    #endif

    engine::ShaderCompiler shaderCompiler;
    shaderCompiler.addShaders("./test/assets/shaders");
    shaderCompiler.compile();

    engine::Window window = engine::Window::Builder()
        .setDimensions(1600, 900)
        .setInitialDisplayMode(engine::Window::DisplayMode::Windowed)
        .setTitle("Testing")
        .build();

    auto windowIcon = asset::decodePng("./muon-logo.png");
    window.setIcon(windowIcon->data, windowIcon->width, windowIcon->height, windowIcon->channels);

    bool mouseGrab{false};

    engine::Device device(window);
    engine::FrameHandler frameHandler(window, device);
    engine::DebugUi debugUi(window, device);

    auto globalPool = engine::DescriptorPool::Builder(device)
        .addPoolSize(vk::DescriptorType::eCombinedImageSampler, std::numeric_limits<int16_t>().max())
        .addPoolSize(vk::DescriptorType::eUniformBuffer, std::numeric_limits<int16_t>().max())
        .buildUniquePtr();

    auto globalSetLayout = engine::DescriptorSetLayout::Builder(device)
        .addBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eAllGraphics, std::numeric_limits<int16_t>().max())
        .addBinding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics, 1)
        .buildUniquePtr();

    auto globalSet = globalSetLayout->createSet(*globalPool);

    struct Ubo {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 transform;
    };

    auto uboBuffer = std::make_unique<engine::Buffer>(
        device,
        sizeof(Ubo),
        1,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vma::MemoryUsage::eCpuCopy
    );

    auto bufferInfo = uboBuffer->getDescriptorInfo();

    engine::DescriptorWriter(*globalPool, *globalSetLayout)
        .addBufferWrite(1, 0, &bufferInfo)
        .writeAll(globalSet);

    std::unique_ptr sceneColor = engine::Image::Builder(device)
        .setExtent(window.getExtent())
        .setFormat(vk::Format::eR8G8B8A8Unorm)
        .setImageUsageFlags(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc)
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setAccessFlags(vk::AccessFlagBits2::eColorAttachmentWrite)
        .setPipelineStageFlags(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
        .buildUniquePtr();

    vk::RenderingAttachmentInfo colorAttachment{};
    colorAttachment.imageView = sceneColor->getImageView();
    colorAttachment.imageLayout = sceneColor->getImageLayout();
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue.color = color::rgbaFromHex<std::array<float, 4>>(0x87ceebff);

    std::unique_ptr sceneDepth = engine::Image::Builder(device)
        .setExtent(window.getExtent())
        .setFormat(vk::Format::eD32Sfloat)
        .setImageUsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment)
        .setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
        .setAccessFlags(vk::AccessFlagBits2::eDepthStencilAttachmentWrite)
        .setPipelineStageFlags(vk::PipelineStageFlagBits2::eEarlyFragmentTests)
        .buildUniquePtr();

    vk::RenderingAttachmentInfo depthAttachment{};
    depthAttachment.imageView = sceneDepth->getImageView();
    depthAttachment.imageLayout = sceneDepth->getImageLayout();
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    depthAttachment.clearValue.depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea = vk::Rect2D{vk::Offset2D{}, window.getExtent()};
    renderingInfo.layerCount = 1;
    renderingInfo.viewMask = 0;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment;
    renderingInfo.pStencilAttachment = nullptr;

    auto sceneColorFormat = sceneColor->getFormat();

    vk::PipelineRenderingCreateInfo renderingCreateInfo{};
    renderingCreateInfo.viewMask = renderingInfo.viewMask;
    renderingCreateInfo.colorAttachmentCount = renderingInfo.colorAttachmentCount;
    renderingCreateInfo.pColorAttachmentFormats = &sceneColorFormat;
    renderingCreateInfo.depthAttachmentFormat = sceneDepth->getFormat();
    renderingCreateInfo.stencilAttachmentFormat = vk::Format::eUndefined;

    RenderSystemTest renderSystem(device, { globalSetLayout->getSetLayout() }, {});
    renderSystem.bake(renderingCreateInfo);

    auto usageFlags = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
    auto accessFlags = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eShaderWrite;

    std::unique_ptr computeImagePool = engine::DescriptorPool::Builder(device)
        .addPoolSize(vk::DescriptorType::eStorageImage, 4)
        .buildUniquePtr();

    std::unique_ptr uiCompositeImage = engine::Image::Builder(device)
        .setExtent(window.getExtent())
        .setFormat(vk::Format::eR8G8B8A8Unorm)
        .setImageUsageFlags(usageFlags)
        .setImageLayout(vk::ImageLayout::eGeneral)
        .setAccessFlags(accessFlags)
        .setPipelineStageFlags(vk::PipelineStageFlagBits2::eComputeShader)
        .buildUniquePtr();

    std::unique_ptr compositeSetLayout = engine::DescriptorSetLayout::Builder(device)
        .addBinding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1)
        .buildUniquePtr();

    auto compositeSet = compositeSetLayout->createSet(*computeImagePool);
    auto compositeInfo = uiCompositeImage->getDescriptorInfo();
    engine::DescriptorWriter(*computeImagePool, *compositeSetLayout)
        .addImageWrite(0, 0, &compositeInfo)
        .writeAll(compositeSet);

    std::unique_ptr computeImageA = engine::Image::Builder(device)
        .setExtent(window.getExtent())
        .setFormat(vk::Format::eR8G8B8A8Unorm)
        .setImageUsageFlags(usageFlags)
        .setImageLayout(vk::ImageLayout::eGeneral)
        .setAccessFlags(accessFlags)
        .setPipelineStageFlags(vk::PipelineStageFlagBits2::eComputeShader)
        .buildUniquePtr();

    std::unique_ptr computeImageB = engine::Image::Builder(device)
        .setExtent(window.getExtent())
        .setFormat(vk::Format::eR8G8B8A8Unorm)
        .setImageUsageFlags(usageFlags)
        .setImageLayout(vk::ImageLayout::eGeneral)
        .setAccessFlags(accessFlags)
        .setPipelineStageFlags(vk::PipelineStageFlagBits2::eComputeShader)
        .buildUniquePtr();

    std::unique_ptr computeSetLayout = engine::DescriptorSetLayout::Builder(device)
        .addBinding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 2)
        .buildUniquePtr();

    auto computeSet = computeSetLayout->createSet(*computeImagePool);
    auto infoA = computeImageA->getDescriptorInfo();
    auto infoB = computeImageB->getDescriptorInfo();
    engine::DescriptorWriter(*computeImagePool, *computeSetLayout)
        .addImageWrite(0, 0, &infoA)
        .addImageWrite(0, 1, &infoB)
        .writeAll(computeSet);

    auto uiComposite = engine::ComputePipeline::Builder(device)
        .setShader("./test/assets/shaders/uicomposite.comp.spv")
        .setDescriptorSetLayouts({ computeSetLayout->getSetLayout(), compositeSetLayout->getSetLayout() })
        .buildUniquePtr();

    auto toneMap = engine::ComputePipeline::Builder(device)
        .setShader("./test/assets/shaders/tonemap.comp.spv")
        .setDescriptorSetLayouts({ computeSetLayout->getSetLayout() })
        .buildUniquePtr();

    auto swizzle = engine::ComputePipeline::Builder(device)
        .setShader("./test/assets/shaders/swizzle.comp.spv")
        .setDescriptorSetLayouts({ computeSetLayout->getSetLayout() })
        .buildUniquePtr();

    bool screenshotRequested{false};

    auto size = window.getExtent().width * window.getExtent().height;

    std::unique_ptr stagingBuffer = std::make_unique<engine::Buffer>(
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
    auto vertexData = engine::Model::getRawVertexData(vertices);

    engine::Model square(device, vertexData, 32, indices);

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
    glm::ivec2 mousePos{0.0, 0.0};
    glm::ivec2 deltaPos{0.0, 0.0};

    float moveSpeed{5.0};

    float rotationSpeed{15.0};
    bool xAxis{false};
    bool yAxis{false};
    bool zAxis{false};

    while (window.isOpen()) {
        float dt = frameHandler.getFrameTime();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            debugUi.pollEvents(&event);

            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                float x, y;
                SDL_GetMouseState(&x, &y);
                mousePos.x = x;
                mousePos.y = y;
                deltaPos.x = event.motion.xrel;
                deltaPos.y = event.motion.yrel;

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
                glm::vec3 forward = orientation;
                forward = glm::normalize(forward);
                glm::vec3 right = glm::normalize(glm::cross(forward, up));
                right = glm::normalize(right);

                glm::vec3 velocity{0.0, 0.0, 0.0};
                if (event.key.scancode == SDL_SCANCODE_W) {
                    velocity += forward;
                }
                if (event.key.scancode == SDL_SCANCODE_S) {
                    velocity -= forward;
                }
                if (event.key.scancode == SDL_SCANCODE_A) {
                    velocity -= right;
                }
                if (event.key.scancode == SDL_SCANCODE_D) {
                    velocity += right;
                }

                if (glm::length(velocity) > 0.0f) {
                    velocity = glm::normalize(velocity);
                    position = position + (velocity * moveSpeed * dt);
                }

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
                    log::globalLogger->info("screenshot requested");
                }
            }

            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                window.resize(event.window.data1, event.window.data2);
            }
        }

        if (seconds >= 1.0) {
            frameRate = frames;
            seconds = 0;
            frames = 0;
        }

        if (window.wasResized()) {
            device.getDevice().waitIdle();

            auto extent = window.getExtent();

            debugUi.recreateSizedResources();

            sceneColor = engine::Image::Builder(device)
                .setExtent(window.getExtent())
                .setFormat(vk::Format::eR8G8B8A8Unorm)
                .setImageUsageFlags(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc)
                .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
                .setAccessFlags(vk::AccessFlagBits2::eColorAttachmentWrite)
                .setPipelineStageFlags(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
                .buildUniquePtr();

            colorAttachment.imageView = sceneColor->getImageView();
            colorAttachment.imageLayout = sceneColor->getImageLayout();
            colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
            colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
            colorAttachment.clearValue.color = color::rgbaFromHex<std::array<float, 4>>(0x87ceebff);

            sceneDepth = engine::Image::Builder(device)
                .setExtent(window.getExtent())
                .setFormat(vk::Format::eD32Sfloat)
                .setImageUsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment)
                .setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
                .setAccessFlags(vk::AccessFlagBits2::eDepthStencilAttachmentWrite)
                .setPipelineStageFlags(vk::PipelineStageFlagBits2::eEarlyFragmentTests)
                .buildUniquePtr();

            depthAttachment.imageView = sceneDepth->getImageView();
            depthAttachment.imageLayout = sceneDepth->getImageLayout();
            depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
            depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
            depthAttachment.clearValue.depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

            renderingInfo.renderArea = vk::Rect2D{vk::Offset2D{}, window.getExtent()};
            renderingInfo.layerCount = 1;
            renderingInfo.viewMask = 0;
            renderingInfo.colorAttachmentCount = 1;
            renderingInfo.pColorAttachments = &colorAttachment;
            renderingInfo.pDepthAttachment = &depthAttachment;
            renderingInfo.pStencilAttachment = nullptr;

            auto sceneColorFormat = sceneColor->getFormat();

            renderingCreateInfo.viewMask = renderingInfo.viewMask;
            renderingCreateInfo.colorAttachmentCount = renderingInfo.colorAttachmentCount;
            renderingCreateInfo.pColorAttachmentFormats = &sceneColorFormat;
            renderingCreateInfo.depthAttachmentFormat = sceneDepth->getFormat();
            renderingCreateInfo.stencilAttachmentFormat = vk::Format::eUndefined;

            renderSystem.bake(renderingCreateInfo);

            uiCompositeImage = engine::Image::Builder(device)
                .setExtent(window.getExtent())
                .setFormat(vk::Format::eR8G8B8A8Unorm)
                .setImageUsageFlags(usageFlags)
                .setImageLayout(vk::ImageLayout::eGeneral)
                .setAccessFlags(accessFlags)
                .setPipelineStageFlags(vk::PipelineStageFlagBits2::eComputeShader)
                .buildUniquePtr();

            auto compositeInfo = uiCompositeImage->getDescriptorInfo();
            engine::DescriptorWriter(*computeImagePool, *compositeSetLayout)
                .addImageWrite(0, 0, &compositeInfo)
                .writeAll(compositeSet);

            computeImageA = engine::Image::Builder(device)
                .setExtent(extent)
                .setFormat(vk::Format::eR8G8B8A8Unorm)
                .setImageUsageFlags(usageFlags)
                .setImageLayout(vk::ImageLayout::eGeneral)
                .setAccessFlags(accessFlags)
                .setPipelineStageFlags(vk::PipelineStageFlagBits2::eComputeShader)
                .buildUniquePtr();

            computeImageB = engine::Image::Builder(device)
                .setExtent(extent)
                .setFormat(vk::Format::eR8G8B8A8Unorm)
                .setImageUsageFlags(usageFlags)
                .setImageLayout(vk::ImageLayout::eGeneral)
                .setAccessFlags(accessFlags)
                .setPipelineStageFlags(vk::PipelineStageFlagBits2::eComputeShader)
                .buildUniquePtr();

            auto infoA = computeImageA->getDescriptorInfo();
            auto infoB = computeImageB->getDescriptorInfo();

            engine::DescriptorWriter(*computeImagePool, *computeSetLayout)
                .addImageWrite(0, 0, &infoA)
                .addImageWrite(0, 1, &infoB)
                .writeAll(computeSet);

            size = extent.width * extent.height;
            stagingBuffer = std::make_unique<engine::Buffer>(
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

        cmd.beginRendering(renderingInfo);

        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(window.getExtent().width);
        viewport.height = static_cast<float>(window.getExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vk::Rect2D scissor{};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent = window.getExtent();

        cmd.setViewport(0, 1, &viewport);
        cmd.setScissor(0, 1, &scissor);

        renderSystem.renderModel(cmd, globalSet, square);

        cmd.endRendering();

        sceneColor->transitionLayout(cmd, {
            .imageLayout = vk::ImageLayout::eTransferSrcOptimal,
            .accessFlags = vk::AccessFlagBits2::eTransferRead,
            .stageFlags = vk::PipelineStageFlagBits2::eTransfer,
        });
        computeImageA->transitionLayout(cmd, {
            .imageLayout = vk::ImageLayout::eTransferDstOptimal,
            .accessFlags = vk::AccessFlagBits2::eTransferWrite,
            .stageFlags = vk::PipelineStageFlagBits2::eTransfer,
        });

        vk::ImageCopy sceneToCompACopy{};
        sceneToCompACopy.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        sceneToCompACopy.srcSubresource.mipLevel = 0;
        sceneToCompACopy.srcSubresource.baseArrayLayer = 0;
        sceneToCompACopy.srcSubresource.layerCount = 1;
        sceneToCompACopy.srcOffset = vk::Offset3D{0, 0, 0};
        sceneToCompACopy.dstSubresource = sceneToCompACopy.srcSubresource;
        sceneToCompACopy.dstOffset = vk::Offset3D{0, 0, 0};
        sceneToCompACopy.extent = vk::Extent3D{window.getExtent(), 1};

        cmd.copyImage(
            sceneColor->getImage(),
            vk::ImageLayout::eTransferSrcOptimal,
            computeImageA->getImage(),
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &sceneToCompACopy
        );

        computeImageA->revertTransition(cmd);
        sceneColor->revertTransition(cmd);

        debugUi.beginRendering(cmd);

        {
            ImGui::Begin("Debug UI");

            ImGui::Text("FPS: %d", frameRate);

            ImGui::Text("Mouse position: %d, %d", mousePos.x, mousePos.y);
            ImGui::Text("Delta position: %d, %d", deltaPos.x, deltaPos.y);

            ImGui::DragFloat("Sensitivity", &sensitivity, 0.1, 0.0, 1.0);
            float oriArray[3] = {orientation.x, orientation.y, orientation.z};
            ImGui::DragFloat3("Orientation", oriArray, 0.01, -1.0, 1.0);
            orientation.x = oriArray[0];
            orientation.y = oriArray[1];
            orientation.z = oriArray[2];

            ImGui::DragFloat("Move speed", &moveSpeed, 0.1, 0.0, 50.0);
            float posArray[3] = {position.x, position.y, position.z};
            ImGui::DragFloat3("Position", posArray);
            position.x = posArray[0];
            position.y = posArray[1];
            position.z = posArray[2];

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

        auto uiImage = debugUi.getImage();

        uiImage->transitionLayout(cmd, {
            .imageLayout = vk::ImageLayout::eTransferSrcOptimal,
            .accessFlags = vk::AccessFlagBits2::eTransferRead,
            .stageFlags = vk::PipelineStageFlagBits2::eTransfer,
        });
        uiCompositeImage->transitionLayout(cmd, {
            .imageLayout = vk::ImageLayout::eTransferDstOptimal,
            .accessFlags = vk::AccessFlagBits2::eTransferWrite,
            .stageFlags = vk::PipelineStageFlagBits2::eTransfer,
        });

        vk::ImageCopy uiToCompositeCopy{};
        uiToCompositeCopy.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        uiToCompositeCopy.srcSubresource.mipLevel = 0;
        uiToCompositeCopy.srcSubresource.baseArrayLayer = 0;
        uiToCompositeCopy.srcSubresource.layerCount = 1;
        uiToCompositeCopy.srcOffset = vk::Offset3D{0, 0, 0};
        uiToCompositeCopy.dstSubresource = uiToCompositeCopy.srcSubresource;
        uiToCompositeCopy.dstOffset = vk::Offset3D{0, 0, 0};
        uiToCompositeCopy.extent = vk::Extent3D{window.getExtent(), 1};

        cmd.copyImage(
            uiImage->getImage(),
            vk::ImageLayout::eTransferSrcOptimal,
            uiCompositeImage->getImage(),
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &uiToCompositeCopy
        );

        uiCompositeImage->revertTransition(cmd);
        uiImage->revertTransition(cmd);

        uiComposite->bind(cmd, { computeSet, compositeSet });
        uiComposite->dispatch(cmd, window.getExtent(), {32, 32, 1});

        toneMap->bind(cmd, { computeSet });
        toneMap->dispatch(cmd, window.getExtent(), {32, 32, 1});

        if (screenshotRequested) {
            computeImageA->transitionLayout(cmd, {
                .imageLayout = vk::ImageLayout::eTransferSrcOptimal,
                .accessFlags = vk::AccessFlagBits2::eTransferRead,
                .stageFlags = vk::PipelineStageFlagBits2::eTransfer,
            });

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
                computeImageA->getImage(),
                vk::ImageLayout::eTransferSrcOptimal,
                stagingBuffer->getBuffer(),
                1,
                &region
            );

            computeImageA->revertTransition(cmd);
        }

        swizzle->bind(cmd, { computeSet });
        swizzle->dispatch(cmd, window.getExtent(), {32, 32, 1});

        computeImageB->transitionLayout(cmd, {
            .imageLayout = vk::ImageLayout::eTransferSrcOptimal,
            .accessFlags = vk::AccessFlagBits2::eTransferRead,
            .stageFlags = vk::PipelineStageFlagBits2::eTransfer,
        });

        frameHandler.copyImageToSwapchain(computeImageB->getImage());

        computeImageB->revertTransition(cmd);

        frameHandler.endFrame();

        if (screenshotRequested) {
            if (stagingBuffer->map() != vk::Result::eSuccess) {
                continue;
            }

            auto extent = window.getExtent();

            asset::Image image{};
            image.width = extent.width;
            image.height = extent.height;
            image.channels = 4;
            image.bitDepth = 8;
            image.data.resize(stagingBuffer->getBufferSize());

            std::memcpy(image.data.data(), stagingBuffer->getMappedMemory(), stagingBuffer->getBufferSize());

            stagingBuffer->unmap();

            std::thread([image]() {
                auto png = asset::encodePng(image);

                std::ofstream outputFile("./screenshot.png");
                outputFile.write(reinterpret_cast<char *>(png->data()), png->size());

                log::globalLogger->info("screenshot saved");
            }).detach();

            screenshotRequested = false;
        }

        frameHandler.updateFrameTiming();

        seconds += frameHandler.getFrameTime();
        frames += 1;
    }

    device.getDevice().waitIdle();
}
