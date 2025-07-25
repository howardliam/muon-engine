#include "muon/input/modifier.hpp"

namespace muon::input {

Modifier::Modifier(const uint8_t &modBitField) : m_mod(modBitField) {}

bool Modifier::isShiftDown() const { return m_mod.test(0); }
bool Modifier::isCtrlDown() const { return m_mod.test(1); }
bool Modifier::isAltDown() const { return m_mod.test(2); }
bool Modifier::isSuperDown() const { return m_mod.test(3); }
bool Modifier::isCapsLockDown() const { return m_mod.test(4); }
bool Modifier::isNumLockDown() const { return m_mod.test(5); }

} // namespace muon::input
