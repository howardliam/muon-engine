#pragma once

#include "muon/engine/utils/nocopy.hpp"
#include "muon/engine/utils/nomove.hpp"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>
#include <memory>

namespace muon {

    class Device;

    class Texture : NoCopy, NoMove {
    public:
        class Builder;

        Texture(
            Device &device,
            const std::vector<uint8_t> &textureData,
            uint32_t channels,
            vk::Extent2D extent
        );
        ~Texture();

        /**
         * @brief   get image descriptor info.
         *
         * @return  descriptor information struct.
         */
        [[nodiscard]] vk::DescriptorImageInfo getDescriptorInfo() const;

    private:
        Device &device;

        vk::Extent2D extent;

        vk::Image image;
        vma::Allocation allocation;
        vk::ImageView imageView;
        vk::Sampler sampler;

        void createTexture(uint32_t channels);
        void prepareForCopying();
        void copyToTexture(const std::vector<uint8_t> &textureData, uint32_t channels);
        void prepareForShader();
    };

    class Texture::Builder {
    public:
        Builder(Device &device);

        Builder &setExtent(vk::Extent2D extent);

        Texture build(const std::vector<uint8_t> &textureData, uint32_t channels) const;
        std::unique_ptr<Texture> buildUniquePtr(const std::vector<uint8_t> &textureData, uint32_t channels) const;

    private:
        Device &device;

        vk::Extent2D extent;
    };

}
