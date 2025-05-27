#pragma once

#include "muon/core/assert.hpp"
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
    };

    struct CloseEventData {};

    using EventData = std::variant<CloseEventData>;

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
