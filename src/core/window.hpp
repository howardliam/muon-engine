#pragma once

#include <cstdint>
#include <filesystem>
#include <SDL3/SDL_video.h>
#include <string_view>
#include <toml++/toml.hpp>
#include <vulkan/vulkan.hpp>

#include "core/logging.hpp"
#include "core/assets/image.hpp"

namespace muon::window {

    class Window {
    public:
        Window(logging::Logger logger, toml::table &config);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        [[nodiscard]] bool createSurface(VkInstance instance, VkSurfaceKHR *surface);

        /* Getters & Setters */
        [[nodiscard]] vk::Extent2D getExtent() const { return { width, height }; }
        [[nodiscard]] bool isOpen() const { return open; }
        void setToClose() { open = false; }

        [[nodiscard]] SDL_Window *getWindow() const { return window; }
        void setTitle(std::string_view newTitle) { SDL_SetWindowTitle(window, newTitle.data()); }
        void setIcon(std::filesystem::path iconPath) {
            assets::PngProperties properties{};
            std::vector<uint8_t> imageData;
            assets::readPngFile(iconPath, imageData, properties);

            SDL_Surface *surface = SDL_CreateSurfaceFrom(properties.width, properties.height, SDL_PIXELFORMAT_RGBA32, imageData.data(), properties.width * 4);
            SDL_SetWindowIcon(window, surface);
        }

    private:
        logging::Logger logger;

        uint32_t width;
        uint32_t height;
        bool open = true;

        SDL_Window *window;

        void initSdl();
        void initWindow(std::string_view title);
    };

}
