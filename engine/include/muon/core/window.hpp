#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace muon {

    class Window2 {
    public:
        struct Properties {
            std::string title;
            uint32_t width;
            uint32_t height;
        };

        Window2();
        ~Window2();

        [[nodiscard]] bool createSurface(VkInstance instance, VkSurfaceKHR *surface);
        [[nodiscard]] std::vector<const char *> requiredExtensions() const;

        [[nodiscard]] vk::Extent2D extent() const;
        [[nodiscard]] uint32_t width() const;
        [[nodiscard]] uint32_t height() const;

    private:
        void initSdl();
        void initWindow(const std::string_view title);

    private:
        struct Handle;

        Handle *handle;

        // GLFWwindow *m_window;

        struct Data {
            std::string title;
            uint32_t width;
            uint32_t height;
        };

        Data m_data;
    };

}
