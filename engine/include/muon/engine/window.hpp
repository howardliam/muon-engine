#pragma once

#include <SDL3/SDL.h>
#include <string_view>

namespace muon::engine::window {

    enum class FullscreenState {
        BorderlessFullscreen,
        Fullscreen,
        Windowed,
    };

    /**
     * @brief Window
    */
    class Window {
    public:
        struct Properties {
            uint32_t width;
            uint32_t height;
            bool open = true;
        };

        /**
         * @brief Base initialiser for Window.
         *
         * @throws program aborts if initialising SDL and window fails.
        */
        explicit Window(Properties &properties);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        /**
         * @brief   Returns the window handle.
         *
         * @return  SDL window handle.
        */
        [[nodiscard]] SDL_Window *getWindow();

        /**
         * @brief   Gets the whether the window is open.
         *
         * @return  boolean value for whether the window is open.
        */
        [[nodiscard]] bool isOpen() const;

        /**
         * @brief   Sets the window to close.
        */
        void setToClose();

    private:
        SDL_Window *window;
        Properties &properties;

        /**
         * @brief Initialises SDL.
         *
         * @throws if SDL failed to initialise.
        */
        void initSdl();

        /**
         * @brief Initialises SDL window.
         *
         * @throws if the window failed to initialise.
        */
        void initWindow(std::string_view title);
    };

}
