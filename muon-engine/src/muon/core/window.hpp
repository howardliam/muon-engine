#pragma once

#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_video.h"
#include "muon/event/dispatcher.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace muon {

enum class WindowMode {
    Windowed,
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

    void pollEvents();

    void requestAttention() const;

    auto createSurface(const vk::raii::Instance &instance) const -> std::optional<vk::raii::SurfaceKHR>;

public:
    auto getTitle() -> const std::string_view;
    void setTitle(const std::string_view title);

    auto getExtent() const -> vk::Extent2D;
    auto getWidth() const -> uint32_t;
    auto getHeight() const -> uint32_t;

    auto getRefreshRate() const -> uint16_t;

    void setMode(WindowMode mode);

    auto getClipboardText() const -> const char *;

    auto getRequiredExtensions() const -> std::vector<const char *>;

private:
    void handleErrors() const;

    void onWindowQuit();
    void onWindowResize(const uint32_t width, const uint32_t height);
    void onWindowFocusChange(const bool focused);

    void onKeyboard(const SDL_Scancode scancode, const bool down, const bool held, const uint16_t mods);

    void onMouseButton(const uint8_t button, const bool down, const uint8_t clicks);
    void onMouseMotion(const float x, const float y);

private:
    std::string m_title;
    vk::Extent2D m_extent;
    uint16_t m_refreshRate;
    WindowMode m_mode;
    const event::Dispatcher &m_dispatcher;

    SDL_Window *m_window;
    SDL_DisplayID m_display;
};

} // namespace muon
