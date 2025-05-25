#include "g_buffer.hpp"

#include <muon/engine/debug/profiler.hpp>
#include <muon/engine/utils/color.hpp>

#include <tracy/TracyVulkan.hpp>

GBufferPass::GBufferPass(muon::Device &device) : device(device) {
    createStaticResources();
}

void GBufferPass::createResources(const vk::Extent2D &extent) {
    albedoImage = muon::Image::Builder(device)
        .setExtent(extent)
        .setFormat(vk::Format::eR8G8B8A8Unorm)
        .setImageUsageFlags(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage)
        .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
        .setAccessFlags(vk::AccessFlagBits2::eColorAttachmentWrite)
        .setPipelineStageFlags(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
        .buildUniquePtr();

    albedoAttachment = std::make_unique<vk::RenderingAttachmentInfo>();
    albedoAttachment->imageView = albedoImage->getImageView();
    albedoAttachment->imageLayout = albedoImage->getImageLayout();
    albedoAttachment->loadOp = vk::AttachmentLoadOp::eClear;
    albedoAttachment->storeOp = vk::AttachmentStoreOp::eStore;
    albedoAttachment->clearValue.color = muon::color::rgbaFromHex<std::array<float, 4>>(0x87ceebff);

    depthImage = muon::Image::Builder(device)
        .setExtent(extent)
        .setFormat(vk::Format::eD32Sfloat)
        .setImageUsageFlags(vk::ImageUsageFlagBits::eDepthStencilAttachment)
        .setImageLayout(vk::ImageLayout::eDepthAttachmentOptimal)
        .setAccessFlags(vk::AccessFlagBits2::eDepthStencilAttachmentWrite)
        .setPipelineStageFlags(vk::PipelineStageFlagBits2::eEarlyFragmentTests)
        .buildUniquePtr();

    depthAttachment = std::make_unique<vk::RenderingAttachmentInfo>();
    depthAttachment->imageView = depthImage->getImageView();
    depthAttachment->imageLayout = depthImage->getImageLayout();
    depthAttachment->loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment->storeOp = vk::AttachmentStoreOp::eStore;
    depthAttachment->clearValue.depthStencil = vk::ClearDepthStencilValue{1.0f, 0};

    renderingInfo = std::make_unique<vk::RenderingInfo>();
    renderingInfo->renderArea = vk::Rect2D{vk::Offset2D{}, extent};
    renderingInfo->layerCount = 1;
    renderingInfo->viewMask = 0;
    renderingInfo->colorAttachmentCount = 1;
    renderingInfo->pColorAttachments = albedoAttachment.get();
    renderingInfo->pDepthAttachment = depthAttachment.get();
    renderingInfo->pStencilAttachment = nullptr;

    auto sceneColorFormat = albedoImage->getFormat();

    renderingCreateInfo = std::make_unique<vk::PipelineRenderingCreateInfo>();
    renderingCreateInfo->viewMask = renderingInfo->viewMask;
    renderingCreateInfo->colorAttachmentCount = renderingInfo->colorAttachmentCount;
    renderingCreateInfo->pColorAttachmentFormats = &sceneColorFormat;
    renderingCreateInfo->depthAttachmentFormat = depthImage->getFormat();
    renderingCreateInfo->stencilAttachmentFormat = vk::Format::eUndefined;

    basicPipeline->bake(*renderingCreateInfo);
}

void GBufferPass::drawFrame(vk::CommandBuffer cmd, const vk::Extent2D &extent, const muon::Mesh &mesh) {
    TracyVkZone(muon::Profiler::context(), cmd, "G-Buffer Pass");

    cmd.beginRendering(*renderingInfo);

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

    basicPipeline->bind(cmd, { globalSet });
    // for (auto &mesh : meshes) {
    // }
    mesh.bind(cmd);
    mesh.draw(cmd);

    cmd.endRendering();
}

muon::DescriptorPool *GBufferPass::getGlobalPool() const {
    return globalPool.get();
}

muon::DescriptorSetLayout *GBufferPass::getGlobalSetLayout() const {
    return globalSetLayout.get();
}

vk::DescriptorSet GBufferPass::getGlobalSet() const {
    return globalSet;
}

muon::Image *GBufferPass::getAlbedoImage() const {
    return albedoImage.get();
}

void GBufferPass::createStaticResources() {
    globalPool = muon::DescriptorPool::Builder(device)
        .addPoolSize(vk::DescriptorType::eCombinedImageSampler, std::numeric_limits<int16_t>().max())
        .addPoolSize(vk::DescriptorType::eUniformBuffer, std::numeric_limits<int16_t>().max())
        .buildUniquePtr();

    globalSetLayout = muon::DescriptorSetLayout::Builder(device)
        .addBinding(0, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eAllGraphics, std::numeric_limits<int16_t>().max())
        .addBinding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics, 1)
        .buildUniquePtr();

    globalSet = globalSetLayout->createSet(*globalPool);

    globalLayout = std::make_shared<muon::PipelineLayout>(device, std::vector{ globalSetLayout->getSetLayout() }, std::nullopt);

    basicPipeline = muon::GraphicsPipeline::Builder(device)
        .addShader(vk::ShaderStageFlagBits::eVertex, "./test/assets/shaders/shader.vert.spv")
        .addShader(vk::ShaderStageFlagBits::eFragment, "./test/assets/shaders/shader.frag.spv")
        .setPipelineLayout(globalLayout)
        .buildUniquePtr();
}
