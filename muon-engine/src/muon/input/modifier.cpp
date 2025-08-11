#include "muon/input/modifier.hpp"

#include "SDL3/SDL_keycode.h"

namespace muon::input {

Modifier::Modifier(const uint16_t bit_field) {
    if (bit_field & SDL_KMOD_SHIFT) {
        m_mod.set(0);
    }

    if (bit_field & SDL_KMOD_CTRL) {
        m_mod.set(1);
    }

    if (bit_field & SDL_KMOD_ALT) {
        m_mod.set(2);
    }

    if (bit_field & SDL_KMOD_GUI) {
        m_mod.set(3);
    }

    if (bit_field & SDL_KMOD_CAPS) {
        m_mod.set(4);
    }

    if (bit_field & SDL_KMOD_NUM) {
        m_mod.set(5);
    }
}

bool Modifier::is_shift_down() const { return m_mod.test(0); }
bool Modifier::is_ctrl_down() const { return m_mod.test(1); }
bool Modifier::is_alt_down() const { return m_mod.test(2); }
bool Modifier::is_super_down() const { return m_mod.test(3); }
bool Modifier::is_caps_lock_down() const { return m_mod.test(4); }
bool Modifier::is_num_lock_down() const { return m_mod.test(5); }

} // namespace muon::input
