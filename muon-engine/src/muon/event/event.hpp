#pragma once

#include "muon/input/key.hpp"
#include "muon/input/modifier.hpp"
#include "muon/input/mouse.hpp"
#include "vulkan/vulkan.hpp"
#include "vulkan/vulkan_structs.hpp"

namespace muon::event {

struct WindowQuitEvent {};

struct WindowResizeEvent {
    vk::Extent2D extent;
};

struct WindowFocusEvent {
    bool focused;
};

struct KeyboardEvent {
    input::Scancode scancode;
    bool down;
    bool held;
    input::Modifier mods;
};

struct MouseButtonEvent {
    input::MouseButton button;
    bool down;
    uint8_t clicks;
};

struct MouseMotionEvent {
    float x;
    float y;
};

struct DropFileEvent {
    const char8_t *path;
};

struct DropTextEvent {
    const char8_t *text;
};

struct TextInputEvent {
    const char8_t *text;
};

} // namespace muon::event
