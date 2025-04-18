#include "muon/engine/window.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <format>
#include <print>
#include <stdexcept>

namespace muon::engine {

    Window::Window(window::Properties &properties) : width(properties.width), height(properties.height) {
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

    void Window::setIcon(std::vector<char> imageData) {
        SDL_Surface *surface = SDL_CreateSurfaceFrom(1000, 1000, SDL_PIXELFORMAT_RGBA32, imageData.data(), 4000);
        bool res = SDL_SetWindowIcon(window, surface);
        if (!res) {
            std::println("{}", SDL_GetError());
        }
    }

    void Window::setDisplayMode(window::DisplayMode mode) {
        bool fullscreen = false;
        bool bordered = false;
        switch (mode) {
            case window::DisplayMode::Windowed:
            fullscreen = false;
            bordered = true;
            break;

            case window::DisplayMode::Fullscreen:
            fullscreen = true;
            bordered = true;
            break;

            case window::DisplayMode::BorderlessFullscreen:
            fullscreen = true;
            bordered = false;
            break;
        }

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

    void Window::initWindow(std::string_view title, window::DisplayMode mode) {
        SDL_WindowFlags modeFlag = 0;
        switch (mode) {
            case window::DisplayMode::Windowed:
            modeFlag |= 0;
            break;

            case window::DisplayMode::Fullscreen:
            modeFlag |= SDL_WINDOW_FULLSCREEN;
            break;

            case window::DisplayMode::BorderlessFullscreen:
            modeFlag |= SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS;
            break;
        }

        SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | modeFlag;

        window = SDL_CreateWindow(title.data(), static_cast<int32_t>(width), static_cast<int32_t>(height), flags);

        if (window == nullptr) {
            throw std::runtime_error(std::format("failed to create window: {}", SDL_GetError()));
        }
    }

}
