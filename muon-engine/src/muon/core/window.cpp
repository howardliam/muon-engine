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
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"
#include "muon/input/key.hpp"
#include "muon/input/modifier.hpp"
#include "muon/input/mouse.hpp"
#include "vulkan/vulkan_raii.hpp"
#include "vulkan/vulkan_structs.hpp"

#include <cstring>
#include <vector>

namespace muon {

struct Window::Impl {
    SDL_Window *window;
    SDL_DisplayID currentDisplay;
};

Window::Window(
    std::string_view title,
    const vk::Extent2D &extent,
    WindowMode mode,
    const event::Dispatcher &dispatcher
) : m_title{title}, m_extent{extent}, m_mode{mode}, m_dispatcher{dispatcher} {
    bool initialized = SDL_Init(SDL_INIT_VIDEO);
    core::expect(initialized, "failed to initialize SDL3: {}", SDL_GetError());

    m_impl = new Impl{};

    m_impl->currentDisplay = SDL_GetPrimaryDisplay();
    core::expect(m_impl->currentDisplay, "failed to get primary display: {}", SDL_GetError());

    const SDL_DisplayMode *displayMode = SDL_GetDesktopDisplayMode(m_impl->currentDisplay);
    core::expect(displayMode, "failed to get desktop display mode: {}", SDL_GetError());
    m_refreshRate = static_cast<uint16_t>(displayMode->refresh_rate);

    core::trace("selected display: {}", SDL_GetDisplayName(m_impl->currentDisplay));

    m_impl->window = SDL_CreateWindow(m_title.c_str(), m_extent.width, m_extent.height, SDL_WINDOW_VULKAN);
    core::expect(m_impl->window, "failed to create window: {}", SDL_GetError());

    bool success = SDL_SetWindowMinimumSize(m_impl->window, displayMode->w / 4, displayMode->h / 4);
    core::expect(success, "failed to set window minimum size: {}", SDL_GetError());

    setMode(m_mode);

    core::debug("created window with dimensions: {}x{}", m_extent.width, m_extent.height);
}

Window::~Window() {
    SDL_DestroyWindow(m_impl->window);
    SDL_Quit();
    core::debug("destroyed window");
}

void Window::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            m_dispatcher.dispatch<event::WindowQuitEvent>({});
        }

        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            m_extent = vk::Extent2D{
                static_cast<uint32_t>(event.window.data1),
                static_cast<uint32_t>(event.window.data2),
            };
            m_dispatcher.dispatch<event::WindowResizeEvent>({m_extent});
        }

        if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
            m_dispatcher.dispatch<event::WindowFocusEvent>({true});
        } else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
            m_dispatcher.dispatch<event::WindowFocusEvent>({false});
        }

        if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            m_dispatcher.dispatch<event::KeyboardEvent>({
                static_cast<input::Scancode>(event.key.scancode),
                event.key.down,
                event.key.repeat,
                input::Modifier{event.key.mod},
            });
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            m_dispatcher.dispatch<event::MouseButtonEvent>({
                static_cast<input::MouseButton>(event.button.button),
                event.button.down,
                event.button.clicks,
            });
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            m_dispatcher.dispatch<event::MouseMotionEvent>({
                event.motion.xrel,
                event.motion.yrel,
            });
        }

        if (event.type == SDL_EVENT_TEXT_INPUT) {
            m_dispatcher.dispatch<event::TextInputEvent>({
                event.text.text,
            });
        }

        if (event.type == SDL_EVENT_DROP_FILE) {
            m_dispatcher.dispatch<event::DropFileEvent>({
                event.drop.data,
            });
        }

        if (event.type == SDL_EVENT_DROP_TEXT) {
            m_dispatcher.dispatch<event::DropTextEvent>({
                event.drop.data,
            });
        }
    }
}

auto Window::getTitle() const -> std::string_view { return m_title; }
void Window::setTitle(std::string_view title) {
    m_title = title;
    SDL_SetWindowTitle(m_impl->window, m_title.c_str());
}

auto Window::getExtent() const -> vk::Extent2D { return m_extent; }
auto Window::getWidth() const -> uint32_t { return m_extent.width; }
auto Window::getHeight() const -> uint32_t { return m_extent.height; }

auto Window::getRefreshRate() const -> uint16_t { return m_refreshRate; }

auto Window::getMode() const -> WindowMode { return m_mode; }
void Window::setMode(WindowMode mode) {
    m_mode = mode;

    bool fullscreen = false;
    switch (m_mode) {
        case WindowMode::Windowed:
            fullscreen = false;
            break;

        case WindowMode::BorderlessFullscreen:
            fullscreen = true;
            break;
    }

    bool success = SDL_SetWindowFullscreen(m_impl->window, fullscreen);
    core::expect(success, "failed to set window mode to: {}, {}", m_mode, SDL_GetError());
}

void Window::requestAttention() const {
    if (!SDL_FlashWindow(m_impl->window, SDL_FLASH_UNTIL_FOCUSED)) {
        handleErrors();
    }
}

auto Window::getClipboardText() const -> std::optional<std::string> {
    if (!SDL_HasClipboardText()) {
        return std::nullopt;
    }

    char *rawText =  SDL_GetClipboardText();
    if (std::strcmp(rawText, "") == 0) {
        SDL_free(rawText);
        return std::nullopt;
    }

    std::string text = rawText;
    SDL_free(rawText);
    return text;
}

void Window::setClipboardText(const std::string_view text) const {
    SDL_SetClipboardText(text.data());
}

void Window::beginTextInput() {
    core::expect(!m_textInput, "cannot begin text input while already accepting text input");

    if (SDL_StartTextInput(m_impl->window)) {
        m_textInput = true;
    } else {
        handleErrors();
    }
}

void Window::endTextInput() {
    core::expect(m_textInput, "cannot end text input while not accepting text input");

    if (SDL_StopTextInput(m_impl->window)) {
        m_textInput = false;
    } else {
        handleErrors();
    }
}

auto Window::getDisplays() const -> std::optional<const std::vector<DisplayInfo>> {
    int32_t displayCount = 0;
    SDL_DisplayID *displays = SDL_GetDisplays(&displayCount);

    if (!displays) {
        return std::nullopt;
    }

    std::vector<DisplayInfo> displayInfos(displayCount);

    for (uint32_t i = 0; i < displayCount; i++) {
        const auto displayId = displays[i];
        DisplayInfo &info = displayInfos[i];

        const char *name = SDL_GetDisplayName(displayId);
        const SDL_DisplayMode *mode = SDL_GetDesktopDisplayMode(displayId);

        if (!name || !mode) {
            return std::nullopt;
        }

        info.name = name;
        info.extent = vk::Extent2D{static_cast<uint32_t>(mode->w), static_cast<uint32_t>(mode->h)};
        info.refreshRate = static_cast<uint16_t>(mode->refresh_rate);
    }

    return displayInfos;
}

auto Window::createSurface(const vk::raii::Instance &instance) const -> std::optional<vk::raii::SurfaceKHR> {
    VkSurfaceKHR surface;

    if (!SDL_Vulkan_CreateSurface(m_impl->window, *instance, nullptr, &surface)) {
        handleErrors();
        return std::nullopt;
    }

    return vk::raii::SurfaceKHR{instance, surface};
}

auto Window::getRequiredExtensions() const -> std::vector<const char *> {
    uint32_t count = 0;
    const char *const *sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&count);
    std::vector<const char *> extensions(sdlExtensions, sdlExtensions + count);
    return extensions;
}

void Window::handleErrors() const {
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
            info.refreshRate
        ),
        ctx
    );
}
