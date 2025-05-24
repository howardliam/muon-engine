#pragma once

#include "muon/engine/utils/nocopy.hpp"
#include "muon/engine/utils/nomove.hpp"
#include <SDL3/SDL.h>
#include <string_view>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace muon {

    class Window : NoCopy, NoMove {
    public:
        enum class DisplayMode;
        struct Properties;
        class Builder;

        explicit Window(const Properties &properties);
        ~Window();

        [[nodiscard]] bool createSurface(VkInstance instance, VkSurfaceKHR *surface);

        [[nodiscard]] SDL_Window *getWindow() const;

        [[nodiscard]] vk::Extent2D getExtent() const;

        [[nodiscard]] bool isOpen() const;

        void setToClose();

        void setTitle(std::string_view title);

        void setIcon(
            std::vector<uint8_t> &imageData,
            const uint32_t width,
            const uint32_t height,
            const uint8_t channels
        );

        void setDisplayMode(DisplayMode mode);

        void resize(uint32_t newWidth, uint32_t newHeight);

        [[nodiscard]] bool wasResized() const;

        void resetResized();

    private:
        SDL_Window *window;

        uint32_t width;
        uint32_t height;
        bool open{true};
        bool resized{false};

        void initSdl();

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
