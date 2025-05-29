#pragma once

#include "muon/core/event.hpp"
#include <string>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

namespace muon {

    struct WindowProperties {
        std::string title;
        uint32_t width;
        uint32_t height;
    };

    class Window {
    public:
        Window(const WindowProperties &props, EventDispatcher *dispatcher);
        ~Window();

        void pollEvents() const;

        [[nodiscard]] vk::Result createSurface(vk::Instance instance, vk::SurfaceKHR *surface);
        [[nodiscard]] std::vector<const char *> requiredExtensions() const;

        [[nodiscard]] void *window() const;

        [[nodiscard]] vk::Extent2D extent() const;
        [[nodiscard]] uint32_t width() const;
        [[nodiscard]] uint32_t height() const;

    private:
        void init();
        void configureDispatcher();

    private:
        GLFWwindow *m_window;

        struct WindowData {
            std::string title;
            uint32_t width;
            uint32_t height;

            EventDispatcher *dispatcher;
        };
        WindowData m_data;
    };

}
