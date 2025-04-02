#include "muon/engine/window.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <format>
#include <print>
#include <stdexcept>

namespace muon::engine::window {

    Window::Window(Window::Properties &properties) : properties(properties) {
        try {
            initSdl();
            initWindow("test");
        } catch (std::exception &e) {
            std::println("{}", e.what());
        }
    }

    Window::~Window() {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    SDL_Window *Window::getWindow() {
        return window;
    }

    bool Window::isOpen() const {
        return properties.open;
    }

    void Window::setToClose() {
        properties.open = false;
    }

    void Window::initSdl() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error(std::format("failed to initialise SDL: {}", SDL_GetError()));
        }
    }

    void Window::initWindow(std::string_view title) {
        SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;

        window = SDL_CreateWindow(title.data(), static_cast<int32_t>(properties.width), static_cast<int32_t>(properties.height), flags);

        if (window == nullptr) {
            throw std::runtime_error(std::format("failed to create window: {}", SDL_GetError()));
        }
    }

}
