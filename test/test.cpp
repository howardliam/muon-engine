#include <memory>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <muon/common/fs.hpp>
#include <muon/engine/buffer.hpp>
#include <muon/engine/descriptor.hpp>
#include <muon/engine/device.hpp>
#include <muon/engine/framebuffer.hpp>
#include <muon/engine/framehandler.hpp>
#include <muon/engine/model.hpp>
#include <muon/engine/pipeline.hpp>
#include <muon/engine/renderpass.hpp>
#include <muon/engine/rendersystem.hpp>
#include <muon/engine/swapchain.hpp>
#include <muon/engine/vertex.hpp>
#include <muon/engine/window.hpp>
#include <muon/assets/image.hpp>
#include <muon/assets/model.hpp>
#include <muon/misc/logger.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>

#include <spdlog/spdlog.h>

#include <vk_mem_alloc_enums.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

using namespace muon;

class Logger : public misc::ILogger {
public:
    Logger() : misc::ILogger() {}

private:
    void traceImpl(std::string message) override {
        spdlog::trace(message);
    }

    void debugImpl(std::string message) override {
        spdlog::debug(message);
    }

    void infoImpl(std::string message) override {
        spdlog::info(message);
    }

    void warnImpl(std::string message) override {
        spdlog::warn(message);
    }

    void errorImpl(std::string message) override {
        spdlog::error(message);
    }
};

struct Vertex {
    glm::vec3 position{};
};

class RenderSystemTest : public engine::RenderSystem {
public:
    RenderSystemTest(
        engine::Device &device,
        std::vector<vk::DescriptorSetLayout> setLayouts,
        vk::RenderPass renderPass
    ) : engine::RenderSystem(device, setLayouts)  {
        createPipeline(renderPass);
    }

    void renderModel(vk::CommandBuffer commandBuffer, vk::DescriptorSet set, const engine::Model<Vertex> &model) {
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

    void createPipeline(vk::RenderPass renderPass) override {
        engine::pipeline::ConfigInfo configInfo;
        engine::pipeline::defaultConfigInfo(configInfo);
        configInfo.renderPass = renderPass;
        configInfo.pipelineLayout = pipelineLayout;

        pipeline = engine::Pipeline::Builder(device)
            .addShader(vk::ShaderStageFlagBits::eVertex, std::filesystem::path("./test/assets/shaders/shader.vert.spv"))
            .addShader(vk::ShaderStageFlagBits::eFragment, std::filesystem::path("./test/assets/shaders/shader.frag.spv"))
            .addVertexAttribute(vk::Format::eR32G32B32Sfloat)
            .buildUniquePointer(configInfo);
    }
};

int main() {
    auto logger = std::make_shared<Logger>();

    engine::window::Properties props;
    props.height = 900;
    props.width = 1600;
    props.mode = engine::window::DisplayMode::Windowed;
    props.title = "Testing";

    engine::Window window(props);
    auto fileData = common::fs::readFile(std::filesystem::path("./muon-logo.png"));
    auto image = assets::loadImagePng(fileData.value());
    window.setIcon(image.data);

    engine::Device device(logger, window);
    engine::FrameHandler frameHandler(window, device);
    frameHandler.setClearColor({0.0f, 0.0f, 0.0f, 1.0f});

    std::unique_ptr pool = engine::DescriptorPool::Builder(device)
        .addPoolSize(vk::DescriptorType::eUniformBuffer, engine::constants::maxFramesInFlight)
        .build();

    struct Ubo {
        glm::mat4 projection;
        glm::mat4 view;
    };

    std::vector<std::unique_ptr<engine::Buffer>> uboBuffers(engine::constants::maxFramesInFlight);
    for (size_t i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<engine::Buffer>(
            device,
            sizeof(Ubo),
            1,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vma::MemoryUsage::eCpuOnly
        );

        auto _ = uboBuffers[i]->map();
    }

    std::unique_ptr setLayout = engine::DescriptorSetLayout::Builder(device)
        .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex)
        .build();

    std::vector<vk::DescriptorSet> descriptorSets(engine::constants::maxFramesInFlight);
    for (size_t i = 0; i < descriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();

        engine::DescriptorWriter(*setLayout, *pool)
            .writeBuffer(0, &bufferInfo)
            .build(descriptorSets[i]);
    }

    std::vector<Vertex> triangleVertices = {
        {{0.0f, -0.5f, -5.0f}},
        {{0.5f, 0.5f, -5.0f}},
        {{-0.5f, 0.5f, -5.0f}},
    };

    engine::Model<Vertex> triangle(device, triangleVertices, {});

    std::vector<Vertex> squareVertices = {
        {{0.5f, 0.5f, -5.0f}},
        {{0.5f, -0.5f, -5.0f}},
        {{-0.5f, -0.5f, -5.0f}},
        {{-0.5f, 0.5f, -5.0f}},
    };

    std::vector<uint32_t> squareIndices = {
        0, 1, 2,
        0, 2, 3,
    };

    engine::Model<Vertex> square(device, squareVertices, squareIndices);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), frameHandler.getAspectRatio(), 0.1f, 1000.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    engine::RenderPass scenePass(device);
    std::unique_ptr sceneFramebuffer = std::make_unique<engine::Framebuffer>(device, scenePass, window.getExtent());

    // RenderSystemTest renderSystem(device, {setLayout->getDescriptorSetLayout()}, frameHandler.getSwapchainRenderPass());
    RenderSystemTest renderSystem(device, {setLayout->getDescriptorSetLayout()}, scenePass.getRenderPass());

    bool screenshotRequested{false};

    while (window.isOpen()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                window.setToClose();
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    window.setToClose();
                }

                if (event.key.scancode == SDL_SCANCODE_F2) {
                    screenshotRequested = true;
                }
            }
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                window.resize(event.window.data1, event.window.data2);

                sceneFramebuffer = std::make_unique<engine::Framebuffer>(device, scenePass, window.getExtent());
            }
        }

        if (const auto commandBuffer = frameHandler.beginFrame()) {
            const int32_t frameIndex = frameHandler.getFrameIndex();

            Ubo ubo{};
            ubo.view = view;
            ubo.projection = projection;

            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            scenePass.beginRenderPass(commandBuffer, sceneFramebuffer->getFramebuffer(), sceneFramebuffer->getExtent());

            renderSystem.renderModel(commandBuffer, descriptorSets[frameIndex], square);

            scenePass.endRenderPass(commandBuffer);

            if (screenshotRequested) {
                auto sceneImage = sceneFramebuffer->getImage();

                auto extent = sceneFramebuffer->getExtent();
                auto size = extent.width * extent.height;

                engine::Buffer stagingBuffer(
                    device,
                    4,
                    size,
                    vk::BufferUsageFlagBits::eTransferDst,
                    vma::MemoryUsage::eGpuToCpu
                );

                device.copyImageToBuffer(sceneImage, stagingBuffer.getBuffer(), extent.width, extent.height, 1);

                assets::ImageData imageData{};
                imageData.width = extent.width;
                imageData.height = extent.height;
                imageData.bitDepth = 8;

                if (stagingBuffer.map() != vk::Result::eSuccess) {
                    logger->error("failed to map screenshot buffer");
                }

                std::vector<char> data(stagingBuffer.getBufferSize());
                std::memcpy(data.data(), stagingBuffer.getMappedMemory(), stagingBuffer.getBufferSize());

                for (size_t i = 0; i < size; i++) {
                    char *pixel = data.data() + (i * 4);
                    std::swap(pixel[0], pixel[2]);
                }

                imageData.data = data;

                std::vector png = assets::encodeImagePng(imageData);
                std::ofstream outputFile("./new.png");
                outputFile.write(reinterpret_cast<char *>(png.data()), png.size());

                logger->info("screenshot saved");

                screenshotRequested = false;
            }

            frameHandler.beginSwapchainRenderPass(commandBuffer);

            /*
                Will actually render despite using a different render pass from the swapchain if
                the render passes are the same, otherwise it will spray out a load of validation
                messages
            */
            renderSystem.renderModel(commandBuffer, descriptorSets[frameIndex], triangle);

            frameHandler.endSwapchainRenderPass(commandBuffer);

            frameHandler.endFrame();
        }
    }

    device.getDevice().waitIdle();
}
