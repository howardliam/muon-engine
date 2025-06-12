#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include "muon/event/dispatcher.hpp"

namespace muon {

    struct WindowProperties {
        std::string title;
        uint32_t width;
        uint32_t height;
    };

    class Window {
    public:
        Window(const WindowProperties &props, event::EventDispatcher *dispatcher);
        ~Window();

        void PollEvents() const;

        [[nodiscard]] VkResult CreateSurface(VkInstance instance, VkSurfaceKHR *surface);
        [[nodiscard]] std::vector<const char *> GetRequiredExtensions() const;

        [[nodiscard]] void *Get() const;

        [[nodiscard]] const char *GetClipboardContents() const;

        [[nodiscard]] VkExtent2D GetExtent() const;
        [[nodiscard]] uint32_t GetWidth() const;
        [[nodiscard]] uint32_t GetHeight() const;

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

            event::EventDispatcher *dispatcher;
        };
        WindowData m_data;
    };

}
