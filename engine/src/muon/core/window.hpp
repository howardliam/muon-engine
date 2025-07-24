#pragma once

#include "muon/event/dispatcher.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace muon {

class Window {
public:
    struct Spec {
        event::Dispatcher *dispatcher{nullptr};
        uint32_t width{std::numeric_limits<uint32_t>().max()};
        uint32_t height{std::numeric_limits<uint32_t>().max()};
        std::string_view title{};
        std::filesystem::path icon{};
    };

public:
    Window(const Spec &spec);
    ~Window();

    auto PollEvents() const -> void;

    auto RequestAttention() const -> void;

    auto CreateSurface(const vk::raii::Instance &instance) const -> std::expected<vk::raii::SurfaceKHR, vk::Result>;

public:
    auto GetExtent() const -> VkExtent2D;
    auto GetWidth() const -> uint32_t;
    auto GetHeight() const -> uint32_t;
    auto GetRefreshRate() const -> uint32_t;
    auto GetRequiredExtensions() const -> std::vector<const char *>;
    auto GetClipboardContents() const -> const char *;

private:
    auto ConfigureDispatchers() -> void;

private:
    struct WindowHandle;
    std::unique_ptr<WindowHandle> m_handle;

    struct WindowData {
        const event::Dispatcher *dispatcher{nullptr};

        std::string title{};
        uint32_t width{};
        uint32_t height{};
        uint32_t refreshRate{};
        bool rawMouseMotion{false};
    };
    WindowData m_data{};
};

} // namespace muon
