#pragma once

#include <muon/engine/renderer/descriptor/pool.hpp>
#include <muon/engine/renderer/descriptor/set_layout.hpp>
#include <muon/engine/renderer/pipeline/graphics.hpp>
#include <muon/engine/renderer/device.hpp>
#include <muon/engine/renderer/image.hpp>
#include <muon/engine/renderer/mesh.hpp>
#include <memory>
#include <vulkan/vulkan.hpp>

using namespace muon;

class GBufferPass {
public:
    GBufferPass(engine::Device &device);

    void createResources(const vk::Extent2D &extent);

    void drawFrame(vk::CommandBuffer cmd, const vk::Extent2D &extent, const engine::Mesh &mesh);

    engine::DescriptorPool *getGlobalPool() const;

    engine::DescriptorSetLayout *getGlobalSetLayout() const;

    vk::DescriptorSet getGlobalSet() const;

    engine::Image *getAlbedoImage() const;

private:
    engine::Device &device;

    std::unique_ptr<engine::DescriptorPool> globalPool;
    std::unique_ptr<engine::DescriptorSetLayout> globalSetLayout;
    vk::DescriptorSet globalSet;
    std::shared_ptr<engine::PipelineLayout> globalLayout;

    std::unique_ptr<engine::Image> albedoImage;
    std::unique_ptr<vk::RenderingAttachmentInfo> albedoAttachment;
    std::unique_ptr<engine::Image> depthImage;
    std::unique_ptr<vk::RenderingAttachmentInfo> depthAttachment;

    std::unique_ptr<vk::RenderingInfo> renderingInfo;

    std::unique_ptr<vk::PipelineRenderingCreateInfo> renderingCreateInfo;
    std::unique_ptr<engine::GraphicsPipeline> basicPipeline;

    void createStaticResources();

};
