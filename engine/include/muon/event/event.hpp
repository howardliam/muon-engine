#pragma once

#include "muon/input/input_state.hpp"
#include "muon/input/key_code.hpp"
#include "muon/input/modifier.hpp"
#include "muon/input/mouse.hpp"
#include <cstdint>
#include <vector>

namespace muon::event {

    struct WindowCloseEvent {};

    struct WindowResizeEvent {
        uint32_t width;
        uint32_t height;
    };

    struct WindowFocusEvent {
        bool focused;
    };

    struct KeyEvent {
        input::KeyCode keycode;
        input::KeyCode scancode;
        input::InputState inputState;
        input::Modifier mods;
    };

    struct MouseButtonEvent {
        input::MouseButton button;
        input::InputState inputState;
        input::Modifier mods;
    };

    struct CursorPositionEvent {
        double x;
        double y;
    };

    struct CursorEnterEvent {
        bool entered;
    };

    struct MouseScrollEvent {
        double xOffset;
        double yOffset;
    };

    struct FileDropEvent {
        std::vector<const char *> paths;
    };

}
