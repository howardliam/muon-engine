#pragma once

#include "fmt/base.h"
#include "muon/core/types.hpp"
#include "muon/event/dispatcher.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace muon {

struct display_info {
    std::string name;
    extent2d extent;
    uint16_t refresh_rate;
};

enum class window_mode {
    windowed,
    borderless_fullscreen,
};

class window {
public:
    window(
        std::string_view title,
        extent2d extent,
        window_mode mode,
        const event::dispatcher &dispatcher
    );
    ~window();

    void poll_events();

public: // class getters/setters
    auto get_title() const -> std::string_view;
    void set_title(std::string_view title);

    auto extent() const -> extent2d;

    auto refresh_rate() const -> uint16_t;

    auto get_mode() const -> window_mode;
    void set_mode(window_mode mode);

public: // underlying SDL API re-exposure
    void request_attention() const;

    auto get_clipboard_text() const -> std::optional<std::string>;
    void set_clipboard_text(std::string_view text);

    void begin_text_input();
    void end_text_input();

    auto displays() const -> std::optional<const std::vector<display_info>>;

public: // Vulkan
    auto create_surface(const vk::raii::Instance &instance) const -> std::optional<vk::raii::SurfaceKHR>;
    auto get_required_extensions() const -> std::vector<const char *>;

private:
    void handle_error() const;

private:
    std::string title_;
    extent2d extent_;
    uint16_t refresh_rate_;
    window_mode mode_;
    const event::dispatcher &dispatcher_;

    struct impl;
    impl *impl_;

    bool text_input_{false};
};

} // namespace muon

template <>
struct fmt::formatter<muon::window_mode> : formatter<string_view> {
    auto format(muon::window_mode mode, format_context &ctx) const -> format_context::iterator;
};

template <>
struct fmt::formatter<muon::display_info> : formatter<string_view> {
    auto format(muon::display_info info, format_context &ctx) const -> format_context::iterator;
};
