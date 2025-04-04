#pragma once

#include <SDL3/SDL.h>
#include <string_view>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    namespace window {
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
    }

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
        explicit Window(window::Properties &properties);
        ~Window();

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        /*
         * @brief   Creates a Vulkan surface for the window.
         *
         * @param   instance    the instance to create the surface for.
         * @param   surface     handle of the surface to be created.
         *
         * @return  boolean value whether it was successful.
        */
        [[nodiscard]] bool createSurface(VkInstance instance, VkSurfaceKHR *surface);

        /**
         * @brief   Returns the window handle.
         *
         * @return  SDL window handle.
        */
        [[nodiscard]] SDL_Window *getWindow() const;

        /**
         * @brief   Returns the window extent.
         *
         * @return  2D vector of the window size.
        */
        [[nodiscard]] vk::Extent2D getExtent() const;

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
         * @param   mode    new display mode.`
        */
        void setDisplayMode(window::DisplayMode mode);

        void resize(uint32_t newWidth, uint32_t newHeight);
        [[nodiscard]] bool wasResized() const;
        void resetResized();

    private:
        SDL_Window *window;

        uint32_t width;
        uint32_t height;
        bool open = true;
        bool resized = false;

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
        void initWindow(std::string_view title, window::DisplayMode mode);
    };

}
