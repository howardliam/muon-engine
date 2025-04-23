#pragma once

#include "muon/engine/device.hpp"
#include "muon/engine/image.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    class Texture {
    public:
        Texture(Device &device);
        ~Texture();

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;


    private:
        Device &device;

        Image image;
        vk::Sampler sampler;

    };

}
