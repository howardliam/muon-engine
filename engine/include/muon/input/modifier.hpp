#pragma once

#include <bitset>
#include <cstdint>

namespace muon::input {

    class Modifier {
    public:
        Modifier(const uint8_t &modBitField);
        ~Modifier() = default;

    public:
        [[nodiscard]] bool IsShiftDown() const;
        [[nodiscard]] bool IsCtrlDown() const;
        [[nodiscard]] bool IsAltDown() const;
        [[nodiscard]] bool IsSuperDown() const;
        [[nodiscard]] bool IsCapsLockDown() const;
        [[nodiscard]] bool IsNumLockDown() const;

    private:
        std::bitset<6> m_mod;
    };

}
