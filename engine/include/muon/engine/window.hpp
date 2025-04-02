#pragma once

#include <SDL3/SDL.h>
#include <string_view>

namespace muon::engine::window {

    enum class FullscreenState {
        BorderlessFullscreen,
        Fullscreen,
        Windowed,
    };

    class Window {
    public:
        struct Properties {
            uint32_t width;
            uint32_t height;
            bool open = true;
        };

        explicit Window(Properties &properties);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        /**
         * @brief   Returns the window handle
         *
         * @return  SDL window handle
        */
        [[nodiscard]] SDL_Window *getWindow() { return window; }

        [[nodiscard]] bool isOpen() const { return properties.open; }
        void setToClose() { properties.open = false; }

    private:
        SDL_Window *window;
        Properties &properties;

        void initSdl();
        void initWindow(std::string_view title);
    };

}
