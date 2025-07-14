#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/graphics/buffer.hpp"
#include "muon/graphics/device_context.hpp"

#include <deque>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

class Texture : NoCopy, NoMove {
public:
    struct Spec {
        const DeviceContext *device{nullptr};
        VkCommandBuffer cmd{nullptr};
        std::deque<Buffer> *uploadBuffers{nullptr};

        VkExtent2D extent{};
        VkFormat format{};
        const std::vector<uint8_t> &textureData{};
        uint32_t pixelSize{};
    };

public:
    Texture(const Spec &spec);
    ~Texture();

public:
    [[nodiscard]] auto Get() const -> VkImage;
    [[nodiscard]] auto GetView() const -> VkImageView;
    [[nodiscard]] auto GetSampler() const -> VkSampler;

    [[nodiscard]] auto GetDescriptorInfo() const -> const VkDescriptorImageInfo &;

private:
    auto CreateImage() -> void;
    auto CreateImageView() -> void;
    auto CreateSampler() -> void;

    auto UploadData(
        VkCommandBuffer cmd, std::deque<Buffer> *uploadBuffers, const std::vector<uint8_t> &textureData, uint32_t pixelSize
    ) -> void;

private:
    const DeviceContext &m_device;

    VkDeviceSize m_bytes{};
    VkExtent2D m_extent{};
    VkFormat m_format{};

    VkImage m_image{nullptr};
    VmaAllocation m_allocation{nullptr};
    VkImageView m_imageView{nullptr};
    VkSampler m_sampler{nullptr};

    VkDescriptorImageInfo m_descriptorInfo{};
};

} // namespace muon::graphics
