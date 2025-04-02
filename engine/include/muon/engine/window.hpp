#pragma once

#include <SDL3/SDL.h>
#include <string_view>

namespace muon::engine::window {

    /**
     * @brief Wrapper for SDL window mode flags.
    */
    enum class DisplayMode {
        Windowed,
        Fullscreen,
        BorderlessFullscreen,
    };

    /**
     * @brief Properties for the window to be initialised with.
    */
    struct Properties {
        uint32_t width;
        uint32_t height;
        DisplayMode mode = DisplayMode::Windowed;
        std::string_view title;
    };

    /**
     * @brief Window.
    */
    class Window {
    public:
        /**
         * @brief   Base initialiser for Window.
         *
         * @throws  program aborts if initialising SDL and window fails.
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
        [[nodiscard]] SDL_Window *getWindow() const;

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

        /**
         * @brief   Sets the window's title.
         *
         * @param   title   new window title.
        */
        void setTitle(std::string_view title);

        /**
         * @brief   Sets the window's display mode.
         *
         * @param   mode    new display mode.
        */
        void setDisplayMode(DisplayMode mode);

    private:
        SDL_Window *window;

        uint32_t width;
        uint32_t height;
        bool open = true;

        /**
         * @brief   Initialises SDL.
         *
         * @throws  if SDL failed to initialise.
        */
        void initSdl();

        /**
         * @brief   Initialises SDL window.
         *
         * @param   title   the title that the window will have.
         * @param   mode    in what mode the window will start in.
         *
         * @throws  if the window failed to initialise.
        */
        void initWindow(std::string_view title, DisplayMode mode);
    };

}
