#pragma once

#include "muon/event/dispatcher.hpp"
#include <string>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

namespace muon {

    struct WindowSpecification {
        std::string_view title;
        uint32_t width;
        uint32_t height;
        event::Dispatcher *dispatcher;
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
        [[nodiscard]] uint32_t GetRefreshRate() const;

        [[nodiscard]] const char *GetClipboardContents() const;

        [[nodiscard]] std::vector<const char *> GetRequiredExtensions() const;

    private:
        void ConfigureDispatchers();

    private:
        GLFWwindow *m_window;

        struct WindowData {
            const event::Dispatcher *dispatcher;

            std::string title;
            uint32_t width;
            uint32_t height;
            uint32_t refreshRate;
            bool rawMouseMotion = false;
        };
        WindowData m_data{};
    };

}
