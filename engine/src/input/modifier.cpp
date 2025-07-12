#include "muon/input/modifier.hpp"

namespace muon::input {

Modifier::Modifier(const uint8_t &modBitField) : m_mod(modBitField) {}

bool Modifier::IsShiftDown() const { return m_mod.test(0); }

bool Modifier::IsCtrlDown() const { return m_mod.test(1); }

bool Modifier::IsAltDown() const { return m_mod.test(2); }

bool Modifier::IsSuperDown() const { return m_mod.test(3); }

bool Modifier::IsCapsLockDown() const { return m_mod.test(4); }

bool Modifier::IsNumLockDown() const { return m_mod.test(5); }

} // namespace muon::input
