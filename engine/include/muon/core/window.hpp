#pragma once

#include "muon/event/dispatcher.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

namespace muon {

    struct WindowSpecification {
        event::Dispatcher *dispatcher;
        uint32_t width;
        uint32_t height;
        std::string_view title;
    };

    class Window {
    public:
        Window(const WindowSpecification &spec);
        ~Window();

        void PollEvents() const;

        [[nodiscard]] VkResult CreateSurface(VkInstance instance, VkSurfaceKHR *surface) const;
        [[nodiscard]] const char *GetClipboardContents() const;
        void RequestAttention() const;

    public:
        [[nodiscard]] GLFWwindow *Get() const;
        [[nodiscard]] VkExtent2D GetExtent() const;
        [[nodiscard]] uint32_t GetWidth() const;
        [[nodiscard]] uint32_t GetHeight() const;
        [[nodiscard]] uint32_t GetRefreshRate() const;
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
