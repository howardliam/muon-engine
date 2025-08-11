#pragma once

#include <bitset>
#include <cstdint>

namespace muon::input {

class Modifier {
public:
    Modifier(const uint16_t bit_field);

    auto is_shift_down() const -> bool;
    auto is_ctrl_down() const -> bool;
    auto is_alt_down() const -> bool;
    auto is_super_down() const -> bool;
    auto is_caps_lock_down() const -> bool;
    auto is_num_lock_down() const -> bool;

private:
    std::bitset<6> m_mod;
};

} // namespace muon::input
