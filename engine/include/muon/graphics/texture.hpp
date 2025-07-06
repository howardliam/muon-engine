#pragma once

#include "muon/graphics/device_context.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    struct TextureSpecification {
        const DeviceContext *device{nullptr};

    };

    class Texture : NoCopy, NoMove {
    public:
        Texture(const TextureSpecification &spec);
        ~Texture();

    public:
        [[nodiscard]] auto Get() const -> VkImage;
        [[nodiscard]] auto GetView() const -> VkImageView;
        [[nodiscard]] auto GetSampler() const -> VkSampler;
        [[nodiscard]] auto GetDescriptorInfo() const -> const VkDescriptorImageInfo &;

    private:
        auto CreateTexture() -> void;

    private:
        const DeviceContext &m_device;

        VkDeviceSize m_bytes;

        VkImage m_image{nullptr};
        VmaAllocation m_allocation{nullptr};
        VkImageView m_imageView{nullptr};
        VkSampler m_sampler{nullptr};

        VkDescriptorImageInfo m_descriptorInfo{};
    };

}
