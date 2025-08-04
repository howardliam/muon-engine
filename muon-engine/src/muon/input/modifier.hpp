#pragma once

#include <bitset>
#include <cstdint>

namespace muon::input {

class Modifier {
public:
    Modifier(const uint16_t modBitField);

public:
    auto isShiftDown() const -> bool;
    auto isCtrlDown() const -> bool;
    auto isAltDown() const -> bool;
    auto isSuperDown() const -> bool;
    auto isCapsLockDown() const -> bool;
    auto isNumLockDown() const -> bool;

private:
    std::bitset<6> m_mod;
};

} // namespace muon::input
