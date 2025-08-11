#pragma once

#include "fmt/base.h"
#include "muon/core/types.hpp"
#include "muon/event/dispatcher.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace muon {

struct DisplayInfo {
    std::string name;
    Extent2D extent;
    uint16_t refresh_rate;
};

enum class WindowMode {
    Windowed,
    BorderlessFullscreen,
};

class Window {
public:
    Window(
        std::string_view title,
        Extent2D extent,
        WindowMode mode,
        const event::Dispatcher &dispatcher
    );
    ~Window();

    void poll_events();

public: // class getters/setters
    auto get_title() const -> std::string_view;
    void set_title(std::string_view title);

    auto extent() const -> Extent2D;

    auto refresh_rate() const -> uint16_t;

    auto get_mode() const -> WindowMode;
    void set_mode(WindowMode mode);

public: // underlying SDL API re-exposure
    void request_attention() const;

    auto get_clipboard_text() const -> std::optional<std::string>;
    void set_clipboard_text(std::string_view text);

    void begin_text_input();
    void end_text_input();

    auto displays() const -> std::optional<const std::vector<DisplayInfo>>;

public: // Vulkan
    // auto create_surface(const vk::raii::Instance &instance) const -> std::optional<vk::raii::SurfaceKHR>;
    auto get_required_extensions() const -> std::vector<const char *>;

private:
    void handle_error() const;

private:
    std::string title_;
    Extent2D extent_;
    uint16_t refresh_rate_;
    WindowMode mode_;
    const event::Dispatcher &dispatcher_;

    struct Impl;
    Impl *impl_;

    bool text_input_{false};
};

} // namespace muon

template <>
struct fmt::formatter<muon::WindowMode> : formatter<string_view> {
    auto format(muon::WindowMode mode, format_context &ctx) const -> format_context::iterator;
};

template <>
struct fmt::formatter<muon::DisplayInfo> : formatter<string_view> {
    auto format(muon::DisplayInfo info, format_context &ctx) const -> format_context::iterator;
};
