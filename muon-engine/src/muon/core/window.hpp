#pragma once

#include "fmt/base.h"
#include "muon/event/dispatcher.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace muon {

struct DisplayInfo {
    std::string name;
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
        std::string_view title,
        const vk::Extent2D &extent,
        WindowMode mode,
        const event::Dispatcher &dispatcher
    );
    ~Window();

    void pollEvents();

public: // class getters/setters
    auto getTitle() const -> std::string_view;
    void setTitle(std::string_view title);

    auto getExtent() const -> vk::Extent2D;
    auto getWidth() const -> uint32_t;
    auto getHeight() const -> uint32_t;

    auto getRefreshRate() const -> uint16_t;

    auto getMode() const -> WindowMode;
    void setMode(WindowMode mode);

public: // underlying SDL API re-exposure
    void requestAttention() const;

    auto getClipboardText() const -> std::optional<std::string>;
    void setClipboardText(std::string_view text) const;

    void beginTextInput();
    void endTextInput();

    auto getDisplays() const -> std::optional<const std::vector<DisplayInfo>>;

public: // Vulkan
    auto createSurface(const vk::raii::Instance &instance) const -> std::optional<vk::raii::SurfaceKHR>;
    auto getRequiredExtensions() const -> std::vector<const char *>;

private:
    void handleErrors() const;

private:
    std::string m_title;
    vk::Extent2D m_extent;
    uint16_t m_refreshRate;
    WindowMode m_mode;
    const event::Dispatcher &m_dispatcher;

    struct Impl;
    Impl *m_impl;

    bool m_textInput{false};
};

} // namespace muon

template <>
struct fmt::formatter<muon::WindowMode> : formatter<string_view> {

    auto format(muon::WindowMode mode, format_context& ctx) const -> format_context::iterator;

};

template <>
struct fmt::formatter<muon::DisplayInfo> : formatter<string_view> {

    auto format(muon::DisplayInfo info, format_context& ctx) const -> format_context::iterator;

};
