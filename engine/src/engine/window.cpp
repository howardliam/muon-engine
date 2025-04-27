#include "muon/engine/window.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <X11/Xlib.h>
#include <format>
#include <print>
#include <stdexcept>

namespace muon::engine {

    Window::Window(const Properties &properties) : width(properties.width), height(properties.height) {
        try {
            initSdl();
            initWindow(properties.title, properties.mode);
        } catch (std::exception &e) {
            std::println("{}", e.what());
        }
    }

    Window::~Window() {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    bool Window::createSurface(VkInstance instance, VkSurfaceKHR *surface) {
        return SDL_Vulkan_CreateSurface(window, instance, nullptr, surface);
    }

    SDL_Window *Window::getWindow() const {
        return window;
    }

    vk::Extent2D Window::getExtent() const {
        return {width, height};
    }

    bool Window::isOpen() const {
        return open;
    }

    void Window::setToClose() {
        open = false;
    }

    void Window::setTitle(std::string_view title) {
        SDL_SetWindowTitle(window, title.data());
    }

    void Window::setIcon(std::vector<uint8_t> imageData, uint32_t width, uint32_t height, uint8_t channels) {
        auto pixelFormatFromChannels = [](uint8_t channels)  {
            switch (channels) {
            case 3:
                return SDL_PIXELFORMAT_RGB24;

            case 4:
                return SDL_PIXELFORMAT_RGBA32;

            default:
                return SDL_PIXELFORMAT_UNKNOWN;
            }
        };

        auto pixelFormat = pixelFormatFromChannels(channels);
        if (pixelFormat == SDL_PIXELFORMAT_UNKNOWN) {
            return;
        }

        SDL_Surface *surface = SDL_CreateSurfaceFrom(width, height, pixelFormat, imageData.data(), width * channels);
        bool res = SDL_SetWindowIcon(window, surface);
        if (!res) {
            std::println("{}", SDL_GetError());
        }
        SDL_DestroySurface(surface);
    }

    void Window::setDisplayMode(DisplayMode mode) {
        auto parseDisplayMode = [](DisplayMode mode) -> std::tuple<bool, bool> {
            switch (mode) {
            case DisplayMode::Windowed:
                return {false, true};

            case DisplayMode::Fullscreen:
                return {true, true};

            case DisplayMode::BorderlessFullscreen:
                return {true, false};
            }

            return {false, false};
        };

        auto [fullscreen, bordered] = parseDisplayMode(mode);

        SDL_SetWindowFullscreen(window, fullscreen);
        SDL_SetWindowBordered(window, bordered);
    }

    void Window::resize(uint32_t newWidth, uint32_t newHeight) {
        width = newWidth;
        height = newHeight;
        resized = true;
    }

    bool Window::wasResized() const {
        return resized;
    }

    void Window::resetResized() {
        resized = false;
    }

    void Window::initSdl() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error(std::format("failed to initialise SDL: {}", SDL_GetError()));
        }
    }

    void Window::initWindow(std::string_view title, DisplayMode mode) {
        SDL_WindowFlags modeFlag = 0;
        switch (mode) {
        case DisplayMode::Windowed:
            modeFlag |= 0;
            break;

        case DisplayMode::Fullscreen:
            modeFlag |= SDL_WINDOW_FULLSCREEN;
            break;

        case DisplayMode::BorderlessFullscreen:
            modeFlag |= SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS;
            break;
        }

        SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | modeFlag;

        window = SDL_CreateWindow(title.data(), static_cast<int32_t>(width), static_cast<int32_t>(height), flags);

        if (window == nullptr) {
            throw std::runtime_error(std::format("failed to create window: {}", SDL_GetError()));
        }
    }

    Window::Builder &Window::Builder::setDimensions(const uint32_t width, const uint32_t height) {
        properties.width = width;
        properties.height = height;
        return *this;
    }

    Window::Builder &Window::Builder::setTitle(const std::string &title) {
        properties.title = title;
        return *this;
    }

    Window::Builder &Window::Builder::setInitialDisplayMode(const DisplayMode displayMode) {
        properties.mode = displayMode;
        return *this;
    }

    Window Window::Builder::build() const {
        return Window(properties);
    }

}
