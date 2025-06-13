#include "muon/input/action.hpp"

namespace muon::input {

    Action::Action(const uint8_t &action) : m_action(static_cast<ActionType>(action)) {}

    bool Action::IsRelease() const {
        return m_action == ActionType::Release;
    }

    bool Action::IsPress() const {
        return m_action == ActionType::Press;
    }

    bool Action::IsRepeat() const {
        return m_action == ActionType::Repeat;
    }

}
