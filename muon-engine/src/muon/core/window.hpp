#pragma once

#include "muon/event/dispatcher.hpp"
#include "vulkan/vulkan_raii.hpp"
#define INCLUDE_GLFW_AFTER_VULKAN
#include "GLFW/glfw3.h"

#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace muon {

enum class WindowMode {
    Windowed,
    Fullscreen,
    BorderlessFullscreen,
};

class Window {
public:
    Window(
        const std::string_view title,
        const vk::Extent2D &extent,
        const WindowMode mode,
        const event::Dispatcher &dispatcher
    );
    ~Window();

    void pollEvents() const;

    void requestAttention() const;

    auto createSurface(const vk::raii::Instance &instance) const -> std::expected<vk::raii::SurfaceKHR, vk::Result>;

public:
    void setMonitor();
    void setMode(WindowMode mode);

    auto getExtent() const -> VkExtent2D;
    auto getWidth() const -> uint32_t;
    auto getHeight() const -> uint32_t;
    auto getRefreshRate() const -> uint32_t;
    auto getRequiredExtensions() const -> std::vector<const char *>;
    auto getClipboardContents() const -> const char *;

private:
    void configureDispatchers();

private:
    GLFWwindow *m_window;
    GLFWmonitor *m_monitor;

    WindowMode m_mode;

    struct Data {
        const event::Dispatcher &dispatcher;

        std::string title;
        vk::Extent2D extent;
        uint32_t refreshRate;
        bool rawMouseMotion{false};

        Data(const event::Dispatcher &dispatcher) : dispatcher{dispatcher} {}
    };
    Data m_data;
};

} // namespace muon
