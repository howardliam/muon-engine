#pragma once

#include "muon/core/types.hpp"
#include "muon/input/key.hpp"
#include "muon/input/modifier.hpp"
#include "muon/input/mouse.hpp"

namespace muon::event {

struct WindowQuit {};

struct WindowResize {
    Extent2D extent;
};

struct WindowFocus {
    bool focused;
};

struct Keyboard {
    input::Scancode scancode;
    bool down;
    bool held;
    input::Modifier mods;
};

struct MouseButton {
    input::MouseButton button;
    bool down;
    uint8_t clicks;
};

struct MouseMotion {
    float x;
    float y;
};

struct DropFile {
    const char *path;
};

struct DropText {
    const char *text;
};

struct TextInput {
    const char *text;
};

} // namespace muon::event
