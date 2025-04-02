#include "muon/engine/window.hpp"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <print>

namespace muon::engine::window {

    Window::Window(Window::Properties &properties) : properties(properties) {
        initSdl();
        initWindow("test");
    }

    Window::~Window() {
        SDL_DestroyWindow(window);
        SDL_Quit();
    }


    void Window::initSdl() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::println("Failed to initialise SDL: {}", SDL_GetError());
        }
    }

    void Window::initWindow(std::string_view title) {
        SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;

        window = SDL_CreateWindow(title.data(), static_cast<int32_t>(properties.width), static_cast<int32_t>(properties.height), flags);

        if (window == nullptr) {
            std::println("Failed to create window: {}", SDL_GetError());
        }
    }

}
