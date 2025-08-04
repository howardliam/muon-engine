#include "muon/core/window.hpp"

#include "SDL3/SDL_clipboard.h"
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_video.h"
#include "SDL3/SDL_vulkan.h"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"
#include "muon/input/key.hpp"
#include "muon/input/modifier.hpp"
#include "muon/input/mouse.hpp"
#include "vulkan/vulkan_raii.hpp"

namespace muon {

Window::Window(
    const std::string_view title,
    const vk::Extent2D &extent,
    const WindowMode mode,
    const event::Dispatcher &dispatcher
) : m_title{title}, m_extent{extent}, m_mode{mode}, m_dispatcher{dispatcher} {
    auto initialized = SDL_Init(SDL_INIT_VIDEO);
    core::expect(initialized, "failed to initialize SDL3");

    m_display = SDL_GetPrimaryDisplay();
    const auto *displayMode = SDL_GetDesktopDisplayMode(m_display);

    m_window = SDL_CreateWindow(m_title.c_str(), m_extent.width, m_extent.height, SDL_WINDOW_VULKAN);
    core::expect(m_window, "window must exist");

    SDL_SetWindowMinimumSize(m_window, displayMode->w / 4, displayMode->h / 4);

    setMode(m_mode);

    core::debug("created window with dimensions: {}x{}", m_extent.width, m_extent.height);
}

Window::~Window() {
    SDL_DestroyWindow(m_window);
    SDL_Quit();
    core::debug("destroyed window");
}

void Window::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            onWindowQuit();
        }

        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            onWindowResize(event.window.data1, event.window.data2);
        }

        if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
            onWindowFocusChange(true);
        } else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
            onWindowFocusChange(false);
        }

        if (event.type == SDL_EVENT_KEY_DOWN) {
            onKeyboard(event.key.scancode, event.key.down, event.key.repeat, event.key.mod);
        } else if (event.type == SDL_EVENT_KEY_UP) {
            onKeyboard(event.key.scancode, event.key.down, event.key.repeat, event.key.mod);
        }

        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            onMouseButton(event.button.button, event.button.down, event.button.clicks);
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            onMouseButton(event.button.button, event.button.down, event.button.clicks);
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            onMouseMotion(event.motion.xrel,event.motion.yrel);
        }
    }
}

void Window::requestAttention() const {
    if (!SDL_FlashWindow(m_window, SDL_FLASH_UNTIL_FOCUSED)) {
        handleErrors();
    }
}

auto Window::createSurface(const vk::raii::Instance &instance) const -> std::optional<vk::raii::SurfaceKHR> {
    VkSurfaceKHR surface;

    if (!SDL_Vulkan_CreateSurface(m_window, *instance, nullptr, &surface)) {
        handleErrors();
        return std::nullopt;
    }

    return vk::raii::SurfaceKHR{instance, surface};
}

auto Window::getTitle() -> const std::string_view { return m_title; }
void Window::setTitle(const std::string_view title) {
    m_title = title;
    SDL_SetWindowTitle(m_window, m_title.c_str());
}

auto Window::getExtent() const -> vk::Extent2D { return m_extent; }
auto Window::getWidth() const -> uint32_t { return m_extent.width; }
auto Window::getHeight() const -> uint32_t { return m_extent.height; }

auto Window::getRefreshRate() const -> uint16_t { return m_refreshRate; }

void Window::setMode(WindowMode mode) {
    switch (mode) {
        case WindowMode::Windowed:
            SDL_SetWindowFullscreen(m_window, false);
            break;

        case WindowMode::BorderlessFullscreen:
            SDL_SetWindowFullscreen(m_window, true);
            break;
    }
}

auto Window::getClipboardText() const -> const char * { return SDL_GetClipboardText(); }

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

void Window::onWindowQuit() {
    m_dispatcher.dispatch<event::WindowQuitEvent>({});
}

void Window::onWindowResize(const uint32_t width, const uint32_t height) {
    m_extent = vk::Extent2D{width, height};
    m_dispatcher.dispatch<event::WindowResizeEvent>({m_extent});
}

void Window::onWindowFocusChange(const bool focused) {
    m_dispatcher.dispatch<event::WindowFocusEvent>({focused});
}

void Window::onKeyboard(const SDL_Scancode scancode, const bool down, const bool held, const uint16_t mods) {
    m_dispatcher.dispatch<event::KeyboardEvent>({
        static_cast<input::Scancode>(scancode),
        down,
        held,
        input::Modifier{mods},
    });
}

void Window::onMouseButton(const uint8_t button, const bool down, const uint8_t clicks) {
    m_dispatcher.dispatch<event::MouseButtonEvent>({
        static_cast<input::MouseButton>(button),
        down,
        clicks,
    });
}

void Window::onMouseMotion(const float x, const float y) {
    m_dispatcher.dispatch<event::MouseMotionEvent>({x, y});
}

} // namespace muon
