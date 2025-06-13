#pragma once

#include <cstdint>
namespace muon::input {

    class Action {
    public:
        Action(const uint8_t &action);
        ~Action() = default;

    public:
        [[nodiscard]] bool IsRelease() const;
        [[nodiscard]] bool IsPress() const;
        [[nodiscard]] bool IsRepeat() const;

    private:
        enum class ActionType {
            Press,
            Release,
            Repeat,
        };

        ActionType m_action;
    };



}
