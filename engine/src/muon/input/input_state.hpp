#pragma once

#include <GLFW/glfw3.h>

namespace muon::input {

enum class InputState {
    Released = GLFW_RELEASE,
    Pressed = GLFW_PRESS,
    Held = GLFW_REPEAT,
};

}
