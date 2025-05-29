#pragma once

#include <variant>
#include <cstdint>
#include "muon/core/input.hpp"

namespace muon {

    struct CloseEventData {};

    struct MouseButtonEventData {
        int32_t button;
        Action action;
        int32_t mods;
    };

    struct MouseScrollEventData {
        double xOffset;
        double yOffset;
    };

    struct KeyEventData {
        int32_t key;
        int32_t scancode;
        Action action;
        int32_t mods;
    };

    struct CursorPositionEventData {
        double x;
        double y;
    };

    using EventData = std::variant<
        CloseEventData,
        MouseButtonEventData,
        MouseScrollEventData,
        KeyEventData,
        CursorPositionEventData
    >;

}
