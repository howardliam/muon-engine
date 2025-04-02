#include "muon/engine/window.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <format>
#include <print>
#include <stdexcept>

namespace muon::engine::window {

    Window::Window(Properties &properties) : width(properties.width), height(properties.height) {
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

    SDL_Window *Window::getWindow() const {
        return window;
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

    void Window::setDisplayMode(DisplayMode mode) {
        bool fullscreen = false;
        bool bordered = false;
        switch (mode) {
            case DisplayMode::Windowed:
            fullscreen = false;
            bordered = true;
            break;

            case DisplayMode::Fullscreen:
            fullscreen = true;
            bordered = true;
            break;

            case DisplayMode::BorderlessFullscreen:
            fullscreen = true;
            bordered = false;
            break;
        }

        SDL_SetWindowFullscreen(window, fullscreen);
        SDL_SetWindowBordered(window, bordered);
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

        SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE | modeFlag;

        window = SDL_CreateWindow(title.data(), static_cast<int32_t>(width), static_cast<int32_t>(height), flags);

        if (window == nullptr) {
            throw std::runtime_error(std::format("failed to create window: {}", SDL_GetError()));
        }
    }

}
