#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/buffer.hpp"
#include "muon/graphics/context.hpp"
#include "vk_mem_alloc.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <deque>

namespace muon::graphics {

class Texture : NoCopy, NoMove {
public:
    struct Spec {
        const Context *context{nullptr};
        vk::raii::CommandBuffer *commandBuffer{nullptr};
        std::deque<Buffer> *uploadBuffers{nullptr};

        vk::Extent2D extent{};
        vk::Format format{};
        const std::vector<uint8_t> &textureData{};
        uint32_t pixelSize{};
    };

public:
    Texture(const Spec &spec);
    ~Texture();

public:
    auto Get() -> vk::raii::Image &;
    auto Get() const -> const vk::raii::Image &;

    auto GetView() -> vk::raii::ImageView &;
    auto GetView() const -> const vk::raii::ImageView &;

    auto GetSampler() -> vk::raii::Sampler &;
    auto GetSampler() const -> const vk::raii::Sampler &;

    auto GetDescriptorInfo() const -> const vk::DescriptorImageInfo &;

private:
    auto CreateImage() -> void;
    auto CreateImageView() -> void;
    auto CreateSampler() -> void;

    auto UploadData(
        vk::raii::CommandBuffer &commandBuffer, std::deque<Buffer> *uploadBuffers, const std::vector<uint8_t> &textureData,
        uint32_t pixelSize
    ) -> void;

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
