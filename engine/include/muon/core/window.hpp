#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace muon {

    class Window {
    public:
        struct Properties {
            std::string title;
            uint32_t width;
            uint32_t height;
        };

        Window(const Properties &props);
        ~Window();

        [[nodiscard]] vk::Result createSurface(vk::Instance instance, vk::SurfaceKHR *surface);
        [[nodiscard]] std::vector<const char *> requiredExtensions() const;

        [[nodiscard]] void *window() const;

        [[nodiscard]] vk::Extent2D extent() const;
        [[nodiscard]] uint32_t width() const;
        [[nodiscard]] uint32_t height() const;

    private:
        struct Impl;
        std::unique_ptr<Impl> m_handle;

        struct Data {
            std::string title;
            uint32_t width;
            uint32_t height;
        };
        Data m_data;
    };

}
