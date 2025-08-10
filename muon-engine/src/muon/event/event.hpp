#pragma once

#include "muon/core/types.hpp"
#include "muon/input/key.hpp"
#include "muon/input/modifier.hpp"
#include "muon/input/mouse.hpp"

namespace muon::event {

struct window_quit_event {};

struct window_resize_event {
    extent2d extent;
};

struct window_focus_event {
    bool focused;
};

struct keyboard_event {
    input::Scancode scancode;
    bool down;
    bool held;
    input::Modifier mods;
};

struct mouse_button_event {
    input::MouseButton button;
    bool down;
    uint8_t clicks;
};

struct mouse_motion_event {
    float x;
    float y;
};

struct drop_file_event {
    const char *path;
};

struct drop_text_event {
    const char *text;
};

struct text_input_event {
    const char *text;
};

} // namespace muon::event
