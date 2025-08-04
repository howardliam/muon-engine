#include "muon/input/modifier.hpp"

#include "SDL3/SDL_keycode.h"

namespace muon::input {

Modifier::Modifier(const uint16_t modBitField) {
    if (modBitField & SDL_KMOD_SHIFT) {
        m_mod.set(0);
    }

    if (modBitField & SDL_KMOD_CTRL) {
        m_mod.set(1);
    }

    if (modBitField & SDL_KMOD_ALT) {
        m_mod.set(2);
    }

    if (modBitField & SDL_KMOD_GUI) {
        m_mod.set(3);
    }

    if (modBitField & SDL_KMOD_CAPS) {
        m_mod.set(4);
    }

    if (modBitField & SDL_KMOD_NUM) {
        m_mod.set(5);
    }
}

bool Modifier::isShiftDown() const { return m_mod.test(0); }
bool Modifier::isCtrlDown() const { return m_mod.test(1); }
bool Modifier::isAltDown() const { return m_mod.test(2); }
bool Modifier::isSuperDown() const { return m_mod.test(3); }
bool Modifier::isCapsLockDown() const { return m_mod.test(4); }
bool Modifier::isNumLockDown() const { return m_mod.test(5); }

} // namespace muon::input
