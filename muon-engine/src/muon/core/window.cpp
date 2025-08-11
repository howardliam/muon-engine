#include "muon/core/window.hpp"

#include "SDL3/SDL_clipboard.h"
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_keyboard.h"
#include "SDL3/SDL_stdinc.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_vulkan.h"
#include "fmt/base.h"
#include "fmt/format.h"
#include "magic_enum/magic_enum.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/core/types.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"
#include "muon/input/key.hpp"
#include "muon/input/modifier.hpp"
#include "muon/input/mouse.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <cstring>
#include <vector>

namespace muon {

struct Window::Impl {
    SDL_Window *window;
    SDL_DisplayID current_display;
};

Window::Window(
    std::string_view title,
    Extent2D extent,
    WindowMode mode,
    const event::Dispatcher &dispatcher
) : title_{title}, extent_{extent}, mode_{mode}, dispatcher_{dispatcher} {
    bool initialized = SDL_Init(SDL_INIT_VIDEO);
    core::expect(initialized, "failed to initialize SDL3: {}", SDL_GetError());

    impl_ = new Impl;

    impl_->current_display = SDL_GetPrimaryDisplay();
    core::expect(impl_->current_display, "failed to get primary display: {}", SDL_GetError());

    const SDL_DisplayMode *displayMode = SDL_GetDesktopDisplayMode(impl_->current_display);
    core::expect(displayMode, "failed to get desktop display mode: {}", SDL_GetError());
    refresh_rate_ = static_cast<uint16_t>(displayMode->refresh_rate);

    core::trace("selected display: {}", SDL_GetDisplayName(impl_->current_display));

    impl_->window = SDL_CreateWindow(title_.c_str(), extent_.width, extent_.height, SDL_WINDOW_VULKAN);
    core::expect(impl_->window, "failed to create window: {}", SDL_GetError());

    bool success = SDL_SetWindowMinimumSize(impl_->window, displayMode->w / 4, displayMode->h / 4);
    core::expect(success, "failed to set window minimum size: {}", SDL_GetError());

    set_mode(mode_);

    core::debug("created window with dimensions: {}x{}", extent_.width, extent_.height);
}

Window::~Window() {
    SDL_DestroyWindow(impl_->window);
    SDL_Quit();
    core::debug("destroyed window");
}

void Window::poll_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            dispatcher_.dispatch<event::WindowQuit>({});
        }

        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            extent_ = Extent2D{
                static_cast<uint32_t>(event.window.data1),
                static_cast<uint32_t>(event.window.data2),
            };
            dispatcher_.dispatch<event::WindowResize>({extent_});
        }

        if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED || event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
            dispatcher_.dispatch<event::WindowFocus>({event.type == SDL_EVENT_WINDOW_FOCUS_GAINED});
        }

        if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            dispatcher_.dispatch<event::Keyboard>({
                static_cast<input::Scancode>(event.key.scancode),
                event.key.down,
                event.key.repeat,
                input::Modifier{event.key.mod},
            });
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            dispatcher_.dispatch<event::MouseButton>({
                static_cast<input::MouseButton>(event.button.button),
                event.button.down,
                event.button.clicks,
            });
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            dispatcher_.dispatch<event::MouseMotion>({
                event.motion.xrel,
                event.motion.yrel,
            });
        }

        if (event.type == SDL_EVENT_TEXT_INPUT) {
            dispatcher_.dispatch<event::TextInput>({
                event.text.text,
            });
        }

        if (event.type == SDL_EVENT_DROP_FILE) {
            dispatcher_.dispatch<event::DropFile>({
                event.drop.data,
            });
        }

        if (event.type == SDL_EVENT_DROP_TEXT) {
            dispatcher_.dispatch<event::DropText>({
                event.drop.data,
            });
        }
    }
}

auto Window::get_title() const -> std::string_view { return title_; }
void Window::set_title(std::string_view title) {
    title_ = title;
    SDL_SetWindowTitle(impl_->window, title_.c_str());
}

auto Window::extent() const -> Extent2D { return extent_; }

auto Window::refresh_rate() const -> uint16_t { return refresh_rate_; }

auto Window::get_mode() const -> WindowMode { return mode_; }
void Window::set_mode(WindowMode mode) {
    mode_ = mode;

    bool fullscreen = false;
    switch (mode_) {
        case WindowMode::Windowed:
            fullscreen = false;
            break;

        case WindowMode::BorderlessFullscreen:
            fullscreen = true;
            break;
    }

    bool success = SDL_SetWindowFullscreen(impl_->window, fullscreen);
    core::expect(success, "failed to set window mode to: {}, {}", mode_, SDL_GetError());
}

void Window::request_attention() const {
    if (!SDL_FlashWindow(impl_->window, SDL_FLASH_UNTIL_FOCUSED)) {
        handle_error();
    }
}

auto Window::get_clipboard_text() const -> std::optional<std::string> {
    if (!SDL_HasClipboardText()) {
        return std::nullopt;
    }

    char *raw_text =  SDL_GetClipboardText();
    if (std::strcmp(raw_text, "") == 0) {
        SDL_free(raw_text);
        return std::nullopt;
    }

    std::string text = raw_text;
    SDL_free(raw_text);
    return text;
}

void Window::set_clipboard_text(const std::string_view text) {
    SDL_SetClipboardText(text.data());
}

void Window::begin_text_input() {
    core::expect(!text_input_, "cannot begin text input while already accepting text input");

    if (SDL_StartTextInput(impl_->window)) {
        text_input_ = true;
    } else {
        handle_error();
    }
}

void Window::end_text_input() {
    core::expect(text_input_, "cannot end text input while not accepting text input");

    if (SDL_StopTextInput(impl_->window)) {
        text_input_ = false;
    } else {
        handle_error();
    }
}

auto Window::displays() const -> std::optional<const std::vector<DisplayInfo>> {
    int32_t display_count = 0;
    SDL_DisplayID *display_ids = SDL_GetDisplays(&display_count);

    if (!display_ids) {
        return std::nullopt;
    }

    std::vector<DisplayInfo> display_infos(display_count);

    for (uint32_t i = 0; i < display_count; i++) {
        const auto display_id = display_ids[i];
        DisplayInfo &info = display_infos[i];

        const char *name = SDL_GetDisplayName(display_id);
        const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(display_id);

        if (!name || !mode) {
            return std::nullopt;
        }

        info.name = name;
        info.extent = Extent2D{static_cast<uint32_t>(mode->w), static_cast<uint32_t>(mode->h)};
        info.refresh_rate = static_cast<uint16_t>(mode->refresh_rate);
    }

    return display_infos;
}

// auto Window::create_surface(const vk::raii::Instance &instance) const -> std::optional<vk::raii::SurfaceKHR> {
//     VkSurfaceKHR surface;

//     if (!SDL_Vulkan_CreateSurface(impl_->window, *instance, nullptr, &surface)) {
//         handle_error();
//         return std::nullopt;
//     }

//     return vk::raii::SurfaceKHR{instance, surface};
// }

auto Window::get_required_extensions() const -> std::vector<const char *> {
    uint32_t count = 0;
    const char *const *sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&count);
    std::vector<const char *> extensions(sdl_extensions, sdl_extensions + count);
    return extensions;
}

void Window::handle_error() const {
    if (const char *error = SDL_GetError(); error) {
        core::error(error);
    }
}

} // namespace muon

auto fmt::formatter<muon::WindowMode>::format(muon::WindowMode mode, format_context& ctx) const -> format_context::iterator {
    return formatter<string_view>::format(magic_enum::enum_name(mode), ctx);
}

auto fmt::formatter<muon::DisplayInfo>::format(muon::DisplayInfo info, format_context& ctx) const -> format_context::iterator {
    return formatter<string_view>::format(
        fmt::format(
            "name: {}, extent: {}x{}, refresh rate: {} Hz",
            info.name,
            info.extent.width,
            info.extent.height,
            info.refresh_rate
        ),
        ctx
    );
}
