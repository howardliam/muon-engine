#pragma once

#include <muon/engine/renderer/descriptor/pool.hpp>
#include <muon/engine/renderer/descriptor/set_layout.hpp>
#include <muon/engine/renderer/pipeline/graphics.hpp>
#include <muon/engine/renderer/device.hpp>
#include <muon/engine/renderer/image.hpp>
#include <muon/engine/renderer/mesh.hpp>
#include <memory>
#include <vulkan/vulkan.hpp>

class GBufferPass {
public:
    GBufferPass(mu::Device &device);

    void createResources(const vk::Extent2D &extent);

    void drawFrame(vk::CommandBuffer cmd, const vk::Extent2D &extent, const mu::Mesh &mesh);

    mu::DescriptorPool *getGlobalPool() const;

    mu::DescriptorSetLayout *getGlobalSetLayout() const;

    vk::DescriptorSet getGlobalSet() const;

    mu::Image *getAlbedoImage() const;

private:
    mu::Device &device;

    std::unique_ptr<mu::DescriptorPool> globalPool;
    std::unique_ptr<mu::DescriptorSetLayout> globalSetLayout;
    vk::DescriptorSet globalSet;
    std::shared_ptr<mu::PipelineLayout> globalLayout;

    std::unique_ptr<mu::Image> albedoImage;
    std::unique_ptr<vk::RenderingAttachmentInfo> albedoAttachment;
    std::unique_ptr<mu::Image> depthImage;
    std::unique_ptr<vk::RenderingAttachmentInfo> depthAttachment;

    std::unique_ptr<vk::RenderingInfo> renderingInfo;

    std::unique_ptr<vk::PipelineRenderingCreateInfo> renderingCreateInfo;
    std::unique_ptr<mu::GraphicsPipeline> basicPipeline;

    void createStaticResources();

};
