#pragma once

#include "muon/input/modifier.hpp"
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
        int32_t action;
        input::Modifier mods;
    };

    struct MouseScrollData {
        double xOffset;
        double yOffset;
    };

    struct KeyData {
        int32_t key;
        int32_t scancode;
        int32_t action;
        input::Modifier mods;
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
