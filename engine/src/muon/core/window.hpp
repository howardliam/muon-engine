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

    auto pollEvents() const -> void;

    auto requestAttention() const -> void;

    auto createSurface(const vk::raii::Instance &instance) const -> std::expected<vk::raii::SurfaceKHR, vk::Result>;

public:
    auto getExtent() const -> VkExtent2D;
    auto getWidth() const -> uint32_t;
    auto getHeight() const -> uint32_t;
    auto getRefreshRate() const -> uint32_t;
    auto getRequiredExtensions() const -> std::vector<const char *>;
    auto getClipboardContents() const -> const char *;

private:
    auto configureDispatchers() -> void;

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
