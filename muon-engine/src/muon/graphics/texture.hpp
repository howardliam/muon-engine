#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/graphics/buffer.hpp"
#include "muon/graphics/context.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <deque>

namespace muon::graphics {

class Texture : NoCopy {
public:
    Texture(
        const Context &context,
        vk::raii::CommandBuffer &commandBuffer,
        std::deque<Buffer> &uploadBuffers,
        const std::vector<uint8_t> &textureData,
        vk::Extent2D extent,
        vk::Format format,
        uint32_t pixelSize
    );
    ~Texture();

    Texture(Texture &&other);
    auto operator=(Texture &&other) -> Texture &;

public:
    auto get() -> vk::raii::Image &;
    auto get() const -> const vk::raii::Image &;

    auto getView() -> vk::raii::ImageView &;
    auto getView() const -> const vk::raii::ImageView &;

    auto getSampler() -> vk::raii::Sampler &;
    auto getSampler() const -> const vk::raii::Sampler &;

    auto getDescriptorInfo() const -> const vk::DescriptorImageInfo &;

private:
    void createImage();
    void createImageView();
    void createSampler();

    void uploadData(
        vk::raii::CommandBuffer &commandBuffer,
        std::deque<Buffer> &uploadBuffers,
        const std::vector<uint8_t> &textureData,
        uint32_t pixelSize
    );

private:
    const Context &m_context;

    vk::DeviceSize m_size;
    vk::Extent2D m_extent;
    vk::Format m_format;

    vk::raii::Image m_image{nullptr};
    vma::Allocation m_allocation{nullptr};
    vk::raii::ImageView m_imageView{nullptr};
    vk::raii::Sampler m_sampler{nullptr};

    vk::DescriptorImageInfo m_descriptorInfo;
};

} // namespace muon::graphics
