#pragma once

#include <cstdint>
#include "muon/core/assert.hpp"
#include "muon/core/event/data.hpp"

namespace muon {

    enum class EventType : uint32_t {
        WindowClose,
        MouseButton,
        MouseScroll,
        Key,
        CursorPosition,
    };

    struct Event {
        EventType type;
        EventData data;

        template<typename T>
        const T &get() const;
    };

    template<typename T>
    const T &Event::get() const {
        MU_CORE_ASSERT(std::holds_alternative<T>(data), "bad variant cast");
        return std::get<T>(data);
    }

    struct EventPolicies {
        static EventType getEvent(const Event &e) {
            return e.type;
        }
    };

}
