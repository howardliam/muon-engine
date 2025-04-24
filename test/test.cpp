#include "muon/engine/pipeline/compute.hpp"
#include "muon/engine/pipeline/graphics.hpp"
#include <memory>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <muon/common/fs.hpp>
#include <muon/engine/buffer.hpp>
#include <muon/engine/computesystem.hpp>
#include <muon/engine/descriptor.hpp>
#include <muon/engine/device.hpp>
#include <muon/engine/framebuffer.hpp>
#include <muon/engine/framehandler.hpp>
#include <muon/engine/image.hpp>
#include <muon/engine/model.hpp>
#include <muon/engine/pipeline.hpp>
#include <muon/engine/renderpass.hpp>
#include <muon/engine/rendersystem.hpp>
#include <muon/engine/swapchain.hpp>
#include <muon/engine/vertex.hpp>
#include <muon/engine/window.hpp>
#include <muon/assets/image.hpp>
#include <muon/misc/logger.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>

#include <spdlog/spdlog.h>

#include <vk_mem_alloc_enums.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>

using namespace muon;

class Logger : public misc::ILogger {
public:
    Logger() : misc::ILogger() {}

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
    void createPipeline(vk::RenderPass renderPass) override {
        engine::GraphicsPipeline::ConfigInfo configInfo;
        engine::GraphicsPipeline::defaultConfigInfo(configInfo);
        configInfo.renderPass = renderPass;
        configInfo.pipelineLayout = pipelineLayout;

        pipeline = engine::GraphicsPipeline::Builder(device)
            .addShader(vk::ShaderStageFlagBits::eVertex, std::filesystem::path("./test/assets/shaders/shader.vert.spv"))
            .addShader(vk::ShaderStageFlagBits::eFragment, std::filesystem::path("./test/assets/shaders/shader.frag.spv"))
            .addVertexAttribute(vk::Format::eR32G32B32Sfloat)
            .buildUniquePointer(configInfo);
    }
};

class ComputeShaderRenderSystem : public engine::ComputeSystem {
public:
    ComputeShaderRenderSystem(
        engine::Device &device,
        std::vector<vk::DescriptorSetLayout> setLayouts
    ) : engine::ComputeSystem(device, setLayouts)  {
        createPipeline();
    }

    void doWork(vk::CommandBuffer commandBuffer, vk::DescriptorSet set) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline->getPipeline());
        commandBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eCompute,
            pipelineLayout,
            0,
            1,
            &set,
            0,
            nullptr
        );

        uint32_t localSize = 32;
        uint32_t dispatchX = (1600 + localSize - 1) / localSize;
        uint32_t dispatchY = (900 + localSize - 1) / localSize;

        commandBuffer.dispatch(1, 1, 1);
    }

protected:
    void createPipeline() override {
        pipeline = std::make_unique<engine::ComputePipeline>(
            device,
            std::filesystem::path("./test/assets/shaders/shader.comp.spv"),
            pipelineLayout
        );
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
        .addPoolSize(vk::DescriptorType::eStorageImage, engine::constants::maxFramesInFlight * 2)
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

    struct TestVertex {
        glm::vec3 position{};
    };
    auto stride = sizeof(TestVertex);

    std::vector<TestVertex> vertices = {
        {{0.5f, 0.5f, -5.0f}},
        {{0.5f, -0.5f, -5.0f}},
        {{-0.5f, -0.5f, -5.0f}},
        {{-0.5f, 0.5f, -5.0f}},
    };

    std::vector vertexData = engine::Model::getRawVertexData(vertices);

    std::vector<uint32_t> indices = {
        0, 1, 2,
        0, 2, 3,
    };

    engine::Model square(device, vertexData, stride, indices);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), frameHandler.getAspectRatio(), 0.1f, 1000.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    engine::RenderPass scenePass(device);
    std::unique_ptr sceneFramebuffer = std::make_unique<engine::Framebuffer>(device, scenePass, window.getExtent());

    RenderSystemTest renderSystem(device, {setLayout->getDescriptorSetLayout()}, scenePass.getRenderPass());

    auto usageFlags = vk::ImageUsageFlagBits::eStorage;
    engine::Image computeImageA(device, window.getExtent(), vk::ImageLayout::eGeneral, vk::Format::eR8G8B8A8Unorm, usageFlags);
    engine::Image computeImageB(device, window.getExtent(), vk::ImageLayout::eGeneral, vk::Format::eR8G8B8A8Unorm, usageFlags);

    std::unique_ptr computeSetLayout = engine::DescriptorSetLayout::Builder(device)
        .addBinding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
        .addBinding(1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
        .build();

    std::vector<vk::DescriptorSet> computeDescriptorSets(engine::constants::maxFramesInFlight);
    for (size_t i = 0; i < descriptorSets.size(); i++) {
        auto infoA = computeImageA.getDescriptorInfo();
        auto infoB = computeImageB.getDescriptorInfo();

        engine::DescriptorWriter(*computeSetLayout, *pool)
            .writeImage(0, &infoA)
            .writeImage(1, &infoB)
            .build(computeDescriptorSets[i]);
    }

    ComputeShaderRenderSystem computeShader(device, {computeSetLayout->getDescriptorSetLayout()});

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

            auto sceneImage = sceneFramebuffer->getImage();

            if (screenshotRequested) {
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

                imageData.data = data;

                std::vector png = assets::encodeImagePng(imageData);
                std::ofstream outputFile("./screenshot.png");
                outputFile.write(reinterpret_cast<char *>(png.data()), png.size());

                logger->info("screenshot saved");

                screenshotRequested = false;
            }

            computeShader.doWork(commandBuffer, computeDescriptorSets[frameIndex]);

            frameHandler.copyImageToSwapchain(sceneImage);

            frameHandler.endFrame();
        }
    }

    device.getDevice().waitIdle();
}
