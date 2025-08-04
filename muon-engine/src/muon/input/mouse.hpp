#pragma once

#include "SDL3/SDL_mouse.h"

namespace muon::input {

enum class MouseButton {
    Button1 = SDL_BUTTON_LEFT,
    Button2 = SDL_BUTTON_RIGHT,
    Button3 = SDL_BUTTON_MIDDLE,
    Button4 = SDL_BUTTON_X1,
    Button5 = SDL_BUTTON_X2,

    Left = Button1,
    Right = Button2,
    Middle = Button3,
};

}
