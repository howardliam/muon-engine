#include "window.hpp"

#include <cstdint>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <exception>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace muon::window {

    namespace defaults {
        constexpr uint32_t width = 800;
        constexpr uint32_t height = 600;
        constexpr std::string_view title = "Default Title";
    }

    Window::Window(logging::Logger logger, toml::table &config) : logger(logger) {
        width = config["window"]["width"].value<uint32_t>().value_or(defaults::width);
        height = config["window"]["height"].value<uint32_t>().value_or(defaults::height);
        auto title = config["window"]["title"].value<std::string_view>().value_or(defaults::title);

        try {
            initSdl();
            initWindow(title);
        } catch (std::exception &e) {
            logger->error(e.what());
            std::terminate();
        }
    }

    Window::~Window() {
        SDL_DestroyWindow(window);
        SDL_Quit();
        logger->debug("Window destroyed");
    }

    bool Window::createSurface(VkInstance instance, VkSurfaceKHR *surface) {
        return SDL_Vulkan_CreateSurface(
            window,
            instance,
            nullptr,
            surface
        );
    }

    void Window::initSdl() {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error(std::format("Failed to initialise SDL: {}", SDL_GetError()));
        }

        logger->debug("Initialised SDL");
    }

    void Window::initWindow(std::string_view title) {
        SDL_WindowFlags flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
        window = SDL_CreateWindow(title.data(), static_cast<int32_t>(width), static_cast<int32_t>(height), flags);

        if (window == nullptr) {
            throw std::runtime_error(std::format("Failed to created window: {}", SDL_GetError()));
        }

        int32_t position = SDL_WINDOWPOS_CENTERED;
        SDL_SetWindowPosition(window, position, position);

        logger->debug("Created window");
    }

}
