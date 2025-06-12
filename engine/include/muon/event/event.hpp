#pragma once

#include "muon/core/assert.hpp"
#include "muon/event/data.hpp"
#include <cstdint>

namespace muon::event {

    enum class EventType : uint32_t {
        WindowClose,
        WindowResize,
        MouseButton,
        MouseScroll,
        Key,
        CursorPosition,
    };

    struct Event {
        EventType type;
        EventData data;

        template<typename T>
        const T &Get() const;
    };

    template<typename T>
    const T &Event::Get() const {
        MU_CORE_ASSERT(std::holds_alternative<T>(data), "bad variant cast");
        return std::get<T>(data);
    }

    struct EventPolicies {
        static EventType getEvent(const Event &e) {
            return e.type;
        }
    };

}
