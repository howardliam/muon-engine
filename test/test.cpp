#include <limits>
#include <memory>
#include <fstream>
#include <chrono>
#include <print>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/trigonometric.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <muon/engine/pipeline/compute.hpp>
#include <muon/engine/pipeline/graphics.hpp>
#include <muon/engine/buffer.hpp>
#include <muon/engine/system/compute.hpp>
#include <muon/engine/descriptor.hpp>
#include <muon/engine/device.hpp>
#include <muon/engine/framebuffer.hpp>
#include <muon/engine/framehandler.hpp>
#include <muon/engine/image.hpp>
#include <muon/engine/model.hpp>
#include <muon/engine/pipeline.hpp>
#include <muon/engine/rendergraph.hpp>
#include <muon/engine/renderpass.hpp>
#include <muon/engine/swapchain.hpp>
#include <muon/engine/vertex.hpp>
#include <muon/engine/window.hpp>
#include <muon/log/logger.hpp>
#include <muon/asset/image.hpp>
#include <muon/engine/system/graphics.hpp>
#include <muon/engine/texture.hpp>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <vk_mem_alloc_enums.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

using namespace muon;

class Logger : public log::ILogger {
public:
    Logger() : log::ILogger() {
        spdlog::set_level(spdlog::level::debug);
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
            .addVertexAttribute(vk::Format::eR32G32B32Sfloat)
            .addVertexAttribute(vk::Format::eR32G32B32Sfloat)
            .addVertexAttribute(vk::Format::eR32G32Sfloat)
            .buildUniquePtr(configInfo);
    }
};

class ToneMap : public engine::ComputeSystem {
public:
    ToneMap(
        engine::Device &device,
        std::vector<vk::DescriptorSetLayout> setLayouts
    ) : engine::ComputeSystem(device, setLayouts) {
        createPipeline();
    }

    void dispatch(
        vk::CommandBuffer commandBuffer,
        vk::DescriptorSet set,
        vk::Extent2D windowExtent,
        const glm::uvec3 &workgroupSize
    ) override {
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

        auto dispatchSize = calculateDispatchSize(windowExtent, workgroupSize);
        commandBuffer.dispatch(dispatchSize.x, dispatchSize.y, dispatchSize.z);
    }

protected:
    void createPipeline() override {
        pipeline = std::make_unique<engine::ComputePipeline>(
            device,
            "./test/assets/shaders/tonemap.comp.spv",
            pipelineLayout
        );
    }
};

class Swizzle : public engine::ComputeSystem {
public:
    Swizzle(
        engine::Device &device,
        std::vector<vk::DescriptorSetLayout> setLayouts
    ) : engine::ComputeSystem(device, setLayouts)  {
        createPipeline();
    }

    void dispatch(
        vk::CommandBuffer commandBuffer,
        vk::DescriptorSet set,
        vk::Extent2D windowExtent,
        const glm::uvec3 &workgroupSize
    ) override {
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

        auto dispatchSize = calculateDispatchSize(windowExtent, workgroupSize);
        commandBuffer.dispatch(dispatchSize.x, dispatchSize.y, dispatchSize.z);
    }

protected:
    void createPipeline() override {
        pipeline = std::make_unique<engine::ComputePipeline>(
            device,
            "./test/assets/shaders/swizzle.comp.spv",
            pipelineLayout
        );
    }
};

int main() {
    auto logger = std::make_shared<Logger>();
    log::setLogger(logger.get());

    engine::Window window = engine::Window::Builder()
        .setDimensions(1600, 900)
        .setInitialDisplayMode(engine::Window::DisplayMode::Windowed)
        .setTitle("Testing")
        .build();

    // auto windowIcon = asset::decodePng("./muon-logo.png");
    // window.setIcon(windowIcon->data, windowIcon->width, windowIcon->height, windowIcon->channels);

    engine::Device device(window);
    engine::FrameHandler frameHandler(window, device);

    std::unique_ptr pool = engine::DescriptorPool::Builder(device)
        .addPoolSize(vk::DescriptorType::eUniformBuffer, engine::constants::maxFramesInFlight)
        .addPoolSize(vk::DescriptorType::eCombinedImageSampler, engine::constants::maxFramesInFlight)
        .build();

    std::unique_ptr globalPool = engine::DescriptorPool::Builder(device)
        .addPoolSize(vk::DescriptorType::eCombinedImageSampler, std::numeric_limits<int16_t>().max())
        .setPoolFlags(vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind)
        .build();

    auto globalSetLayout = engine::DescriptorSetLayout::Builder(device)
        .addBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eAllGraphics, std::numeric_limits<int16_t>().max())
        .setFlags(vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool)
        .setBindless(true)
        .build();

    struct Ubo {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 transform;
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
        .addBinding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics)
        // .addBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eAllGraphics)
        .build();

    std::vector<vk::DescriptorSet> descriptorSets(engine::constants::maxFramesInFlight);
    for (size_t i = 0; i < descriptorSets.size(); i++) {
        auto bufferInfo = uboBuffers[i]->descriptorInfo();

        engine::DescriptorWriter(*setLayout, *pool)
            .writeBuffer(0, &bufferInfo)
            .build(descriptorSets[i]);
    }

    std::unique_ptr sceneColor = engine::Image::Builder(device)
        .setExtent(window.getExtent())
        .setFormat(vk::Format::eR8G8B8A8Unorm)
        .setImageUsageFlags(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc)
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setAccessFlags(vk::AccessFlagBits::eColorAttachmentWrite)
        .setPipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .buildUniquePtr();

    vk::RenderingAttachmentInfo colorAttachment{};
    colorAttachment.imageView = sceneColor->getImageView();
    colorAttachment.imageLayout = sceneColor->getImageLayout();
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue.color = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f};

    std::unique_ptr sceneDepth = engine::Image::Builder(device)
        .setExtent(window.getExtent())
        .setFormat(vk::Format::eD32Sfloat)
        .setImageUsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment)
        .setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
        .setAccessFlags(vk::AccessFlagBits::eDepthStencilAttachmentWrite)
        .setPipelineStageFlags(vk::PipelineStageFlagBits::eEarlyFragmentTests)
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

    RenderSystemTest renderSystem(device, { setLayout->getDescriptorSetLayout() }, {});
    renderSystem.bake(renderingCreateInfo);

    auto usageFlags = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
    auto accessFlags = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;

    std::unique_ptr computeImageA = engine::Image::Builder(device)
        .setExtent(window.getExtent())
        .setFormat(vk::Format::eR8G8B8A8Unorm)
        .setImageUsageFlags(usageFlags)
        .setImageLayout(vk::ImageLayout::eGeneral)
        .setAccessFlags(accessFlags)
        .setPipelineStageFlags(vk::PipelineStageFlagBits::eComputeShader)
        .buildUniquePtr();

    std::unique_ptr computeImageB = engine::Image::Builder(device)
        .setExtent(window.getExtent())
        .setFormat(vk::Format::eR8G8B8A8Unorm)
        .setImageUsageFlags(usageFlags)
        .setImageLayout(vk::ImageLayout::eGeneral)
        .setAccessFlags(accessFlags)
        .setPipelineStageFlags(vk::PipelineStageFlagBits::eComputeShader)
        .buildUniquePtr();

    std::unique_ptr computeImagePool = engine::DescriptorPool::Builder(device)
        .addPoolSize(vk::DescriptorType::eStorageImage, 4)
        .build();

    std::unique_ptr computeSetLayout = engine::DescriptorSetLayout::Builder(device)
        .addBinding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
        .addBinding(1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute)
        .build();

    auto infoA = computeImageA->getDescriptorInfo();
    auto infoB = computeImageB->getDescriptorInfo();

    std::array<vk::DescriptorSet, 2> computeDescriptorSets;

    engine::DescriptorWriter(*computeSetLayout, *computeImagePool)
        .writeImage(0, &infoA)
        .writeImage(1, &infoB)
        .build(computeDescriptorSets[0]);

    engine::DescriptorWriter(*computeSetLayout, *computeImagePool)
        .writeImage(0, &infoB)
        .writeImage(1, &infoA)
        .build(computeDescriptorSets[1]);

    ToneMap tonemap(device, {computeSetLayout->getDescriptorSetLayout()});
    Swizzle swizzle(device, {computeSetLayout->getDescriptorSetLayout()});

    bool screenshotRequested{false};

    auto extent = window.getExtent();
    auto size = extent.width * extent.height;

    std::unique_ptr stagingBuffer = std::make_unique<engine::Buffer>(
        device,
        4,
        size,
        vk::BufferUsageFlagBits::eTransferDst,
        vma::MemoryUsage::eGpuToCpu
    );

    glm::mat4 transform = glm::mat4{1.0f};
    transform = glm::scale(transform, glm::vec3{2.0f});
    transform = glm::rotate(transform, glm::radians(30.0f), glm::vec3{1.0f, 1.0f, 0.0f});

    auto currentTime = std::chrono::high_resolution_clock::now();
    float frameTime{0.0};

    float seconds{0.0};
    int32_t frames{0};

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

    int32_t frameIndex{0};

    engine::RenderGraph renderGraph;

    renderGraph.addImage(
        "SceneColor",
        engine::Image::Builder(device)
            .setExtent(window.getExtent())
            .setFormat(vk::Format::eR8G8B8A8Unorm)
            .setImageUsageFlags(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc)
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setAccessFlags(vk::AccessFlagBits::eColorAttachmentWrite)
            .setPipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .buildUniquePtr()
    );

    renderGraph.addNode({
        .name = "SceneRender",
        .nodeType = engine::RenderGraph::NodeType::Graphics,

        .readResources = {},
        .writeResources = {
            { "SceneColor", vk::ImageLayout::eColorAttachmentOptimal, vk::AccessFlagBits::eColorAttachmentWrite, vk::PipelineStageFlagBits::eColorAttachmentOutput },
            { "SceneDepth", vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::PipelineStageFlagBits::eEarlyFragmentTests },
        },

        .compile = []() {

        },
        .execute = [&](vk::CommandBuffer cmd, engine::RenderGraph::FrameInfo frameInfo) {
            cmd.beginRendering(renderingInfo);

            vk::Viewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(extent.width);
            viewport.height = static_cast<float>(extent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            vk::Rect2D scissor{};
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            scissor.extent = extent;

            cmd.setViewport(0, 1, &viewport);
            cmd.setScissor(0, 1, &scissor);

            renderSystem.renderModel(cmd, descriptorSets[frameIndex], square);

            cmd.endRendering();
        }
    });

    renderGraph.addNode({
        .name = "ToneMap",
        .nodeType = engine::RenderGraph::NodeType::Compute,

        .readResources = {
            { "SceneColor", vk::ImageLayout::eTransferSrcOptimal, vk::AccessFlagBits::eTransferRead, vk::PipelineStageFlagBits::eTransfer },
        },
        .writeResources = {
            { "ToneMapOutput", vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite, vk::PipelineStageFlagBits::eComputeShader },
        },

        .compile = []() {

        },
        .execute = [&](vk::CommandBuffer cmd, engine::RenderGraph::FrameInfo frameInfo) {
            sceneColor->transitionLayout(cmd, {
                .imageLayout = vk::ImageLayout::eTransferSrcOptimal,
                .accessFlags = vk::AccessFlagBits::eTransferRead,
                .pipelineStageFlags = vk::PipelineStageFlagBits::eTransfer,
            });

            computeImageA->transitionLayout(cmd, {
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

            cmd.copyImage(
                sceneColor->getImage(),
                vk::ImageLayout::eTransferSrcOptimal,
                computeImageA->getImage(),
                vk::ImageLayout::eTransferDstOptimal,
                1,
                &imageCopy
            );

            computeImageA->revertTransition(cmd);

            sceneColor->revertTransition(cmd);

            tonemap.dispatch(cmd, computeDescriptorSets[frameInfo.pingPongIndex], window.getExtent(), {32, 32, 1});
        }
    });

    renderGraph.addNode({
        .name = "Swizzle",
        .nodeType = engine::RenderGraph::NodeType::Compute,

        .readResources = {
            { "ToneMapOutput", vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eComputeShader },
        },
        .writeResources = {
            { "SwizzleOutput", vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite, vk::PipelineStageFlagBits::eComputeShader },
        },

        .compile = []() {

        },
        .execute = [&](vk::CommandBuffer cmd, engine::RenderGraph::FrameInfo frameInfo) {
            swizzle.dispatch(cmd, computeDescriptorSets[frameInfo.pingPongIndex], window.getExtent(), {32, 32, 1});
        }
    });

    renderGraph.addNode({
        .name = "FinalPresentation",
        .nodeType = engine::RenderGraph::NodeType::Transfer,

        .readResources = {
            { "SwizzleOutput", vk::ImageLayout::eTransferSrcOptimal, vk::AccessFlagBits::eTransferRead, vk::PipelineStageFlagBits::eTransfer },
        },
        .writeResources = {
            { "SwapchainImage", vk::ImageLayout::eTransferDstOptimal, vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eTransfer },
        },

        .compile = []() {

        },
        .execute = [&](vk::CommandBuffer cmd, engine::RenderGraph::FrameInfo frameInfo) {
            computeImageA->transitionLayout(cmd, {
                .imageLayout = vk::ImageLayout::eTransferSrcOptimal,
                .accessFlags = vk::AccessFlagBits::eTransferRead,
                .pipelineStageFlags = vk::PipelineStageFlagBits::eTransfer,
            });

            frameHandler.copyImageToSwapchain(computeImageA->getImage());

            computeImageA->revertTransition(cmd);
        }
    });

    renderGraph.compile();

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
                    // screenshotRequested = true;
                    log::globalLogger->info("screenshots disabled");
                }
            }
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                window.resize(event.window.data1, event.window.data2);
            }
        }

        if (seconds >= 1.0) {
            std::string title = std::format("FPS: {}", frames);
            window.setTitle(title);
            seconds = 0;
            frames = 0;
        }

        if (window.wasResized()) {
            device.getDevice().waitIdle();

            extent = window.getExtent();

            computeImageA = engine::Image::Builder(device)
                .setExtent(extent)
                .setFormat(vk::Format::eR8G8B8A8Unorm)
                .setImageUsageFlags(usageFlags)
                .setImageLayout(vk::ImageLayout::eGeneral)
                .setAccessFlags(accessFlags)
                .setPipelineStageFlags(vk::PipelineStageFlagBits::eComputeShader)
                .buildUniquePtr();

            computeImageB = engine::Image::Builder(device)
                .setExtent(extent)
                .setFormat(vk::Format::eR8G8B8A8Unorm)
                .setImageUsageFlags(usageFlags)
                .setImageLayout(vk::ImageLayout::eGeneral)
                .setAccessFlags(accessFlags)
                .setPipelineStageFlags(vk::PipelineStageFlagBits::eComputeShader)
                .buildUniquePtr();

            computeImagePool->resetPool();

            auto infoA = computeImageA->getDescriptorInfo();
            auto infoB = computeImageB->getDescriptorInfo();

            engine::DescriptorWriter(*computeSetLayout, *computeImagePool)
                .writeImage(0, &infoA)
                .writeImage(1, &infoB)
                .build(computeDescriptorSets[0]);

            engine::DescriptorWriter(*computeSetLayout, *computeImagePool)
                .writeImage(0, &infoB)
                .writeImage(1, &infoA)
                .build(computeDescriptorSets[1]);

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

        const auto commandBuffer = frameHandler.beginFrame();

        frameIndex = frameHandler.getFrameIndex();

        transform = glm::rotate(transform, glm::tau<float>() * frameTime, glm::vec3{1.0f, 1.0f, 1.0f});

        Ubo ubo{};
        ubo.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.projection = glm::perspective(glm::radians(45.0f), frameHandler.getAspectRatio(), 0.1f, 1000.0f);
        ubo.transform = transform;

        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();

        renderGraph.execute(commandBuffer);

        frameHandler.endFrame();

        // if (screenshotRequested) {
        //     computeImageB->transitionLayout(commandBuffer, {
        //         .imageLayout = vk::ImageLayout::eTransferSrcOptimal,
        //         .accessFlags = vk::AccessFlagBits::eTransferRead,
        //         .pipelineStageFlags = vk::PipelineStageFlagBits::eTransfer,
        //     });

        //     auto extent = window.getExtent();

        //     vk::BufferImageCopy region{};
        //     region.bufferOffset = 0;
        //     region.bufferRowLength = 0;
        //     region.bufferImageHeight = 0;
        //     region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        //     region.imageSubresource.mipLevel = 0;
        //     region.imageSubresource.baseArrayLayer = 0;
        //     region.imageSubresource.layerCount = 1;
        //     region.imageOffset.x = 0;
        //     region.imageOffset.y = 0;
        //     region.imageOffset.z = 0;
        //     region.imageExtent.width = extent.width;
        //     region.imageExtent.height = extent.height;
        //     region.imageExtent.depth = 1;

        //     commandBuffer.copyImageToBuffer(
        //         computeImageB->getImage(),
        //         vk::ImageLayout::eTransferSrcOptimal,
        //         stagingBuffer->getBuffer(),
        //         1,
        //         &region
        //     );

        //     computeImageB->revertTransition(commandBuffer);
        // }

        // if (screenshotRequested) {
        //     if (stagingBuffer->map() != vk::Result::eSuccess) {
        //         continue;
        //     }

        //     asset::Image image{};
        //     image.width = extent.width;
        //     image.height = extent.height;
        //     image.channels = 4;
        //     image.bitDepth = 8;
        //     image.data.resize(stagingBuffer->getBufferSize());

        //     std::memcpy(image.data.data(), stagingBuffer->getMappedMemory(), stagingBuffer->getBufferSize());

        //     stagingBuffer->unmap();

        //     std::thread([image]() {
        //         auto png = asset::encodePng(image);

        //         std::ofstream outputFile("./screenshot.png");
        //         outputFile.write(reinterpret_cast<char *>(png->data()), png->size());

        //         log::globalLogger->info("screenshot saved");
        //     }).detach();

        //     screenshotRequested = false;
        // }

        auto newTime = std::chrono::high_resolution_clock::now();
        frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        seconds += frameTime;
        frames += 1;
    }

    device.getDevice().waitIdle();
}
