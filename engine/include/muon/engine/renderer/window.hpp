#pragma once

#include "muon/engine/utils/nocopy.hpp"
#include "muon/engine/utils/nomove.hpp"
#include <SDL3/SDL.h>
#include <string_view>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    /**
     * @brief   Wrapper around SDL window.
     */
    class Window : NoCopy, NoMove {
    public:
        enum class DisplayMode;
        struct Properties;
        class Builder;

        /**
         * @brief   Base initialiser for Window.
         *
         * @throws  program aborts if initialising SDL and window fails.
         */
        explicit Window(const Properties &properties);
        ~Window();

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
         * @brief   Sets the window's icon.
         *
         * @param   imageData   vector of bytes corresponding to the image data.
         * @param   width       width of the image in pixels.
         * @param   height      height of the image in pixels.
         * @param   channels    number of channels in the image; RGB = 3, RGBA = 4, etc.
         */
        void setIcon(
            std::vector<uint8_t> &imageData,
            const uint32_t width,
            const uint32_t height,
            const uint8_t channels
        );

        /**
         * @brief   Sets the window's display mode.
         *
         * @param   mode    new display mode.
         */
        void setDisplayMode(DisplayMode mode);

        /**
         * @brief   Resizes the window.
         *
         * @param   newWidth    new width.
         * @param   newHeight   new height.
         */
        void resize(uint32_t newWidth, uint32_t newHeight);

        /**
         * @brief   Checks if the window was resized.
         *
         * @return  whether the window was resized.
         */
        [[nodiscard]] bool wasResized() const;

        /**
         * @brief   Resets the window resized flag to false.
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
        /**
         * @brief   Sets the window's dimensions.
         *
         * @param   width    the width of the window.
         * @param   height   the height of the window.
         *
         * @return  reference to the builder.
         */
        Builder &setDimensions(const uint32_t width, const uint32_t height);

        /**
         * @brief   Sets the window's title.
         *
         * @param   title    the title of the window.
         *
         * @return  reference to the builder.
         */
        Builder &setTitle(std::string_view title);

        /**
         * @brief   Sets the window's initial display mode.
         *
         * @param   displayMode the display mode: windowed, borderless, fullscreen, etc.
         *
         * @return  reference to the builder.
         */
        Builder &setInitialDisplayMode(const DisplayMode &displayMode);

        /**
         * @brief   Builds a new window class from configuration.
         *
         * @return  the newly constructed window.
         */
        Window build() const;

    private:
        Properties properties{};
    };

}
