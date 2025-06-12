#pragma once

#include "muon/core/input.hpp"
#include <variant>
#include <cstdint>

namespace muon::event {

    struct WindowCloseData {};

    struct WindowResizeData {
        uint32_t width;
        uint32_t height;
    };

    struct MouseButtonData {
        int32_t button;
        Action action;
        int32_t mods;
    };

    struct MouseScrollData {
        double xOffset;
        double yOffset;
    };

    struct KeyData {
        int32_t key;
        int32_t scancode;
        Action action;
        int32_t mods;
    };

    struct CursorPositionData {
        double x;
        double y;
    };

    using EventData = std::variant<
        WindowCloseData,
        WindowResizeData,
        MouseButtonData,
        MouseScrollData,
        KeyData,
        CursorPositionData
    >;

}
