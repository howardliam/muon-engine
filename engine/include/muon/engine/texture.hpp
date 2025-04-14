#pragma once

#include "muon/engine/device.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    class Texture {
    public:
        Texture(Device &device);
        ~Texture();

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;

        [[nodiscard]] vk::DescriptorImageInfo getDescriptorInfo() const;
        [[nodiscard]] vk::Sampler getSampler() const;
        [[nodiscard]] vk::ImageView getImageView() const;
        [[nodiscard]] vk::ImageLayout getImageLayout() const;

        [[nodiscard]] uint32_t getWidth() const;
        [[nodiscard]] uint32_t getHeight() const;

    private:
        Device &device;

        uint32_t width{0};
        uint32_t height{0};

        vk::Image image;
        vma::Allocation allocation;
        vk::Sampler sampler;
        vk::ImageView imageView;
        vk::ImageLayout imageLayout;
        vk::Format format;
        uint32_t instanceSize{0};

        void createTexture();
        void transitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    };

}
