#pragma once

#include "muon/input/modifier.hpp"
#include <cstdint>

namespace muon::event {

    struct WindowCloseEvent {};

    struct WindowResizeEvent {
        uint32_t width;
        uint32_t height;
    };

    struct MouseButtonEvent {
        int32_t button;
        int32_t action;
        input::Modifier mods;
    };

    struct MouseScrollEvent {
        double xOffset;
        double yOffset;
    };

    struct KeyEvent {
        int32_t key;
        int32_t scancode;
        int32_t action;
        input::Modifier mods;
    };

    struct CursorPositionEvent {
        double x;
        double y;
    };

}
