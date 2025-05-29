#pragma once

#include "muon/core/assert.hpp"
#include "muon/core/event_data.hpp"
#include <variant>
#include <cstdint>
#include <eventpp/eventdispatcher.h>

namespace muon {

    enum class EventType : int32_t {
        WindowClose,
        MouseButton,
        MouseScroll,
        Key,
        CursorPosition,
    };

    using EventData = std::variant<
        CloseEventData,
        MouseButtonEventData,
        MouseScrollEventData,
        KeyEventData,
        CursorPositionEventData
    >;

    struct Event {
        EventType type;
        EventData data;

        template<typename T>
        const T &get() const;
    };

    struct EventPolicies {
        static EventType getEvent(const Event &e) {
            return e.type;
        }
    };

    template<typename T>
    const T &Event::get() const {
        MU_CORE_ASSERT(std::holds_alternative<T>(data), "bad variant cast");
        return std::get<T>(data);
    }

    using EventDispatcher = eventpp::EventDispatcher<EventType, void (const Event &), EventPolicies>;

}
