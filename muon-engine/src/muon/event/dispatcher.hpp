#pragma once

#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"

#include <eventpp/eventdispatcher.h>
#include <typeindex>

namespace muon::event {

class Dispatcher : NoCopy, NoMove {
private:
    using EventDispatcher = eventpp::EventDispatcher<std::type_index, void(const void *)>;

public:
    using Handle = EventDispatcher::Handle;

public:
    template <typename Event>
    auto subscribe(std::function<void(const Event &)> listener) -> Handle {
        return m_dispatcher.appendListener(typeid(Event), [listener](const void *event) {
            listener(*static_cast<const Event *>(event));
        });
    }

    template <typename Event>
    [[nodiscard]] auto unsubscribe(const Handle &handle) -> bool {
        return m_dispatcher.removeListener(typeid(Event), handle);
    }

    template <typename Event>
    auto dispatch(const Event &event) const -> void {
        m_dispatcher.dispatch(typeid(Event), &event);
    }

private:
    EventDispatcher m_dispatcher;
};

} // namespace muon::event
