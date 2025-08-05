#pragma once

#include "muon/event/dispatcher.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace muon {

struct DisplayInfo {
    std::u8string name;
    vk::Extent2D extent;
    uint16_t refreshRate;
};

enum class WindowMode {
    Windowed,
    BorderlessFullscreen,
};

class Window {
public:
    Window(
        const std::u8string_view title,
        const vk::Extent2D &extent,
        const WindowMode mode,
        const event::Dispatcher &dispatcher
    );
    ~Window();

    void pollEvents();

public: // class getters/setters
    auto getTitle() -> const std::u8string_view;
    void setTitle(const std::u8string_view title);

    auto getExtent() const -> vk::Extent2D;
    auto getWidth() const -> uint32_t;
    auto getHeight() const -> uint32_t;

    auto getRefreshRate() const -> uint16_t;

    void setMode(WindowMode mode);

public: // underlying SDL API re-exposure
    void requestAttention() const;

    auto getClipboardText() const -> std::optional<std::u8string>;
    void setClipboardText(const std::u8string_view text) const;

    void beginTextInput();
    void endTextInput();

    auto getDisplays() const -> std::optional<const std::vector<DisplayInfo>>;

public: // Vulkan
    auto createSurface(const vk::raii::Instance &instance) const -> std::optional<vk::raii::SurfaceKHR>;
    auto getRequiredExtensions() const -> std::vector<const char *>;

private:
    void handleErrors() const;

private:
    std::u8string m_title;
    vk::Extent2D m_extent;
    uint16_t m_refreshRate;
    WindowMode m_mode;
    const event::Dispatcher &m_dispatcher;

    struct Impl;
    Impl *m_impl;

    bool m_textInput{false};
};

} // namespace muon
