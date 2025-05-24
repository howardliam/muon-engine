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
    GBufferPass(muon::Device &device);

    void createResources(const vk::Extent2D &extent);

    void drawFrame(vk::CommandBuffer cmd, const vk::Extent2D &extent, const muon::Mesh &mesh);

    muon::DescriptorPool *getGlobalPool() const;

    muon::DescriptorSetLayout *getGlobalSetLayout() const;

    vk::DescriptorSet getGlobalSet() const;

    muon::Image *getAlbedoImage() const;

private:
    muon::Device &device;

    std::unique_ptr<muon::DescriptorPool> globalPool;
    std::unique_ptr<muon::DescriptorSetLayout> globalSetLayout;
    vk::DescriptorSet globalSet;
    std::shared_ptr<muon::PipelineLayout> globalLayout;

    std::unique_ptr<muon::Image> albedoImage;
    std::unique_ptr<vk::RenderingAttachmentInfo> albedoAttachment;
    std::unique_ptr<muon::Image> depthImage;
    std::unique_ptr<vk::RenderingAttachmentInfo> depthAttachment;

    std::unique_ptr<vk::RenderingInfo> renderingInfo;

    std::unique_ptr<vk::PipelineRenderingCreateInfo> renderingCreateInfo;
    std::unique_ptr<muon::GraphicsPipeline> basicPipeline;

    void createStaticResources();

};
