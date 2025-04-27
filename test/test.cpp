
#include "muon/engine/framegraph.hpp"
#include <memory>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <muon/engine/pipeline/compute.hpp>
#include <muon/engine/pipeline/graphics.hpp>
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
#include <muon/assets/file.hpp>
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
        std::vector<vk::DescriptorSetLayout> setLayouts
    ) : engine::RenderSystem(device, setLayouts)  {

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
            .addShader(vk::ShaderStageFlagBits::eVertex, "./test/assets/shaders/shader.vert.spv")
            .addShader(vk::ShaderStageFlagBits::eFragment, "./test/assets/shaders/shader.frag.spv")
            .addVertexAttribute(vk::Format::eR32G32B32Sfloat)
            .buildUniquePtr(configInfo);
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

        commandBuffer.dispatch(dispatchX, dispatchY, 1);
    }

protected:
    void createPipeline() override {
        pipeline = std::make_unique<engine::ComputePipeline>(
            device,
            "./test/assets/shaders/shader.comp.spv",
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
    auto image = assets::loadImage("./muon-logo.png");
    window.setIcon(image->data, image->size.width, image->size.height, 4);

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

    engine::RenderPass scenePass = engine::RenderPass::Builder(device)
        .addColorAttachment(vk::Format::eR8G8B8A8Unorm)
        .addDepthStencilAttachment(vk::Format::eD32Sfloat)
        .build();

    std::unique_ptr sceneFramebuffer = std::make_unique<engine::Framebuffer>(device, scenePass.getRenderPass(), scenePass.getAttachments(), window.getExtent());

    RenderSystemTest renderSystem(device, {setLayout->getDescriptorSetLayout()});
    renderSystem.bake(scenePass.getRenderPass());

    auto usageFlags = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
    auto accessFlags = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;

    engine::Image computeImageA = engine::Image::Builder(device)
        .setExtent(window.getExtent())
        .setFormat(vk::Format::eR8G8B8A8Unorm)
        .setImageUsageFlags(usageFlags)
        .setImageLayout(vk::ImageLayout::eGeneral)
        .setAccessFlags(accessFlags)
        .setPipelineStageFlags(vk::PipelineStageFlagBits::eComputeShader)
        .build();

    engine::Image computeImageB = engine::Image::Builder(device)
        .setExtent(window.getExtent())
        .setFormat(vk::Format::eR8G8B8A8Unorm)
        .setImageUsageFlags(usageFlags)
        .setImageLayout(vk::ImageLayout::eGeneral)
        .setAccessFlags(accessFlags)
        .setPipelineStageFlags(vk::PipelineStageFlagBits::eComputeShader)
        .build();

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

    auto extent = window.getExtent();
    auto size = extent.width * extent.height;

    engine::Buffer stagingBuffer(
        device,
        4,
        size,
        vk::BufferUsageFlagBits::eTransferDst,
        vma::MemoryUsage::eGpuToCpu
    );

    bool resizeRequested{false};

    // engine::FrameGraph frameGraph;

    // frameGraph.addStage({
    //     .name = "SceneStage",
    //     .readResources = {},
    //     .writeResources = {},

    //     .compile = [=]() {

    //     },

    //     .record = [=](vk::CommandBuffer commandBuffer) {

    //     }
    // });

    // frameGraph.compile();

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
                    logger->warn("screenshot requested");
                }
            }
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                resizeRequested = true;
                window.resize(event.window.data1, event.window.data2);
            }
        }

        const auto commandBuffer = frameHandler.beginFrame();

        const int32_t frameIndex = frameHandler.getFrameIndex();

        Ubo ubo{};
        ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.projection = glm::perspective(glm::radians(45.0f), frameHandler.getAspectRatio(), 0.1f, 1000.0f);

        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();

        scenePass.begin(commandBuffer, sceneFramebuffer->getFramebuffer(), sceneFramebuffer->getExtent());

        renderSystem.renderModel(commandBuffer, descriptorSets[frameIndex], square);

        scenePass.end(commandBuffer);

        auto sceneImage = sceneFramebuffer->getImage(0);

        (*sceneImage)->transitionLayout(commandBuffer, {
            .imageLayout = vk::ImageLayout::eTransferSrcOptimal,
            .accessFlags = vk::AccessFlagBits::eTransferRead,
            .pipelineStageFlags = vk::PipelineStageFlagBits::eTransfer,
        });

        computeImageA.transitionLayout(commandBuffer, {
            .imageLayout = vk::ImageLayout::eTransferDstOptimal,
            .accessFlags = vk::AccessFlagBits::eTransferWrite,
            .pipelineStageFlags = vk::PipelineStageFlagBits::eTransfer,
        });

        vk::ImageCopy imageCopy{};
        imageCopy.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        imageCopy.srcSubresource.mipLevel = 0;
        imageCopy.srcSubresource.baseArrayLayer = 0;
        imageCopy.srcSubresource.layerCount = 1;
        imageCopy.srcOffset = vk::Offset3D{0, 0, 0};
        imageCopy.dstSubresource = imageCopy.srcSubresource;
        imageCopy.dstOffset = vk::Offset3D{0, 0, 0};
        imageCopy.extent.width = window.getExtent().width;
        imageCopy.extent.height = window.getExtent().height;
        imageCopy.extent.depth = 1;

        commandBuffer.copyImage(
            (*sceneImage)->getImage(),
            vk::ImageLayout::eTransferSrcOptimal,
            computeImageA.getImage(),
            vk::ImageLayout::eTransferDstOptimal,
            1,
            &imageCopy
        );

        computeImageA.revertTransition(commandBuffer);

        (*sceneImage)->revertTransition(commandBuffer);

        computeShader.doWork(commandBuffer, computeDescriptorSets[frameIndex]);

        computeImageB.transitionLayout(commandBuffer, {
            .imageLayout = vk::ImageLayout::eTransferSrcOptimal,
            .accessFlags = vk::AccessFlagBits::eTransferRead,
            .pipelineStageFlags = vk::PipelineStageFlagBits::eTransfer,
        });

        if (screenshotRequested) {
            vk::BufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset.x = 0;
            region.imageOffset.y = 0;
            region.imageOffset.z = 0;
            region.imageExtent.width = extent.width;
            region.imageExtent.height = extent.height;
            region.imageExtent.depth = 1;

            commandBuffer.copyImageToBuffer(
                computeImageB.getImage(),
                vk::ImageLayout::eTransferSrcOptimal,
                stagingBuffer.getBuffer(),
                1,
                &region
            );
        }

        frameHandler.copyImageToSwapchain(computeImageB.getImage());

        computeImageB.revertTransition(commandBuffer);

        frameHandler.endFrame();

        if (resizeRequested) {
            sceneFramebuffer = std::make_unique<engine::Framebuffer>(device, scenePass.getRenderPass(), scenePass.getAttachments(), window.getExtent());

            resizeRequested = false;
        }

        if (screenshotRequested) {
            if (stagingBuffer.map() != vk::Result::eSuccess) {
                logger->error("failed to map screenshot buffer");
            }

            std::vector<uint8_t> data(stagingBuffer.getBufferSize());
            std::memcpy(data.data(), stagingBuffer.getMappedMemory(), stagingBuffer.getBufferSize());

            assets::Image image{};
            image.size = {extent.width, extent.height};
            image.format = assets::ColorFormat::Rgba;
            image.bitDepth = 8;
            image.data = data;

            auto png = assets::encodeImage(image, assets::ImageFormat::Png);

            std::ofstream outputFile("./screenshot.png");
            outputFile.write(reinterpret_cast<char *>(png->data()), png->size());

            logger->warn("screenshot saved");

            screenshotRequested = false;
        }
    }

    device.getDevice().waitIdle();
}
