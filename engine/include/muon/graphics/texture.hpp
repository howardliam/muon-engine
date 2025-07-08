#pragma once

#include "muon/graphics/device_context.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <vector>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {
    class Texture : NoCopy, NoMove {
    public:
        struct Spec {
            const DeviceContext *device{nullptr};
            VkExtent2D extent{};
            VkFormat format{};
            const std::vector<uint8_t> &textureData{};
            uint32_t pixelSize{};
            VkCommandBuffer cmd{nullptr};
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

        auto UploadData(VkCommandBuffer cmd, const std::vector<uint8_t> &textureData, uint32_t pixelSize) -> void;

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

}
