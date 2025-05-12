#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <SDL3/SDL.h>
#include <string_view>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    /**
     * @brief   wrapper around SDL window.
     */
    class Window : NoCopy, NoMove {
    public:
        enum class DisplayMode;
        struct Properties;
        class Builder;

        /**
         * @brief   base initialiser for Window.
         *
         * @throws  program aborts if initialising SDL and window fails.
         */
        explicit Window(const Properties &properties);
        ~Window();

        /*
         * @brief   creates a Vulkan surface for the window.
         *
         * @param   instance    the instance to create the surface for.
         * @param   surface     handle of the surface to be created.
         *
         * @return  boolean value whether it was successful.
         */
        [[nodiscard]] bool createSurface(VkInstance instance, VkSurfaceKHR *surface);

        /**
         * @brief   returns the window handle.
         *
         * @return  SDL window handle.
         */
        [[nodiscard]] SDL_Window *getWindow() const;

        /**
         * @brief   returns the window extent.
         *
         * @return  2D vector of the window size.
         */
        [[nodiscard]] vk::Extent2D getExtent() const;

        /**
         * @brief   gets the whether the window is open.
         *
         * @return  boolean value for whether the window is open.
         */
        [[nodiscard]] bool isOpen() const;

        /**
         * @brief   sets the window to close.
         */
        void setToClose();

        /**
         * @brief   sets the window's title.
         *
         * @param   title   new window title.
         */
        void setTitle(std::string_view title);

        /**
         * @brief   sets the window's icon.
         *
         * @param   imageData   vector of bytes corresponding to the image data.
         */
        void setIcon(
            std::vector<uint8_t> &imageData,
            const uint32_t width,
            const uint32_t height,
            const uint8_t channels
        );

        /**
         * @brief   sets the window's display mode.
         *
         * @param   mode    new display mode.
         */
        void setDisplayMode(DisplayMode mode);

        /**
         * @brief   resizes the window.
         *
         * @param   newWidth    new width.
         * @param   newHeight   new height.
         */
        void resize(uint32_t newWidth, uint32_t newHeight);

        /**
         * @brief   checks if the window was resized.
         *
         * @return  whether the window was resized.
         */
        [[nodiscard]] bool wasResized() const;

        /**
         * @brief   resets the window resized to false.
         */
        void resetResized();

    private:
        SDL_Window *window;

        uint32_t width;
        uint32_t height;
        bool open{true};
        bool resized{false};

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
        void initWindow(std::string_view title, const DisplayMode &mode);
    };

    enum class Window::DisplayMode {
        Windowed,
        Fullscreen,
        BorderlessFullscreen,
    };

    struct Window::Properties {
        uint32_t width{1280};
        uint32_t height{720};
        DisplayMode mode{DisplayMode::Windowed};
        std::string_view title{"Muon"};
    };

    class Window::Builder {
    public:
        Builder &setDimensions(const uint32_t width, const uint32_t height);

        Builder &setTitle(std::string_view title);

        Builder &setInitialDisplayMode(const DisplayMode &displayMode);

        Window build() const;

    private:
        Properties properties{};
    };

}
