#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include "muon/core/event/dispatcher.hpp"

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

        void PollEvents() const;

        [[nodiscard]] VkResult CreateSurface(VkInstance instance, VkSurfaceKHR *surface);
        [[nodiscard]] std::vector<const char *> RequiredExtensions() const;

        [[nodiscard]] void *Get() const;

        [[nodiscard]] const char *ClipboardContents() const;

        [[nodiscard]] VkExtent2D Extent() const;
        [[nodiscard]] uint32_t Width() const;
        [[nodiscard]] uint32_t Height() const;

    private:
        void Init();
        void ConfigureDispatcher();

    private:
        GLFWwindow *m_window;

        struct WindowData {
            std::string title;
            uint32_t width;
            uint32_t height;
            bool rawMouseMotion;

            EventDispatcher *dispatcher;
        };
        WindowData m_data;
    };

}
