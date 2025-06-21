#pragma once

#include "muon/event/dispatcher.hpp"
#include <string>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

namespace muon {

    struct WindowSpecification {
        std::string title;
        uint32_t width;
        uint32_t height;
        event::EventDispatcher *dispatcher;
    };

    class Window {
    public:
        Window(const WindowSpecification &props);
        ~Window();

        void PollEvents() const;

        [[nodiscard]] VkResult CreateSurface(VkInstance instance, VkSurfaceKHR *surface) const;

    public:
        [[nodiscard]] GLFWwindow *Get() const;

        [[nodiscard]] VkExtent2D GetExtent() const;
        [[nodiscard]] uint32_t GetWidth() const;
        [[nodiscard]] uint32_t GetHeight() const;

        [[nodiscard]] const char *GetClipboardContents() const;

        [[nodiscard]] std::vector<const char *> GetRequiredExtensions() const;

    private:
        void ConfigureDispatchers();

    private:
        GLFWwindow *m_window;

        struct WindowData {
            std::string title;
            uint32_t width;
            uint32_t height;

            const event::EventDispatcher *dispatcher;
        };
        WindowData m_data;
    };

}
