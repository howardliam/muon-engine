#pragma once

#include "muon/core/assert.hpp"
#include "muon/core/input.hpp"
#include <variant>
#include <cstdint>
#include <eventpp/eventdispatcher.h>

namespace muon {

    enum class EventType;
    struct Event;
    struct EventPolicies;

    using EventDispatcher = eventpp::EventDispatcher<EventType, void (const Event &), EventPolicies>;

    enum class EventType : int32_t {
        WindowClose,
        MouseButton,
    };

    struct CloseEventData {};

    struct MouseButtonEventData {
        int32_t button;
        Action action;
        int32_t mods;
    };

    using EventData = std::variant<CloseEventData, MouseButtonEventData>;

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

}
