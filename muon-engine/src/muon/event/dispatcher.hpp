#pragma once

#include "muon/utils/no_copy.hpp"
#include "muon/utils/no_move.hpp"

#include <eventpp/eventdispatcher.h>
#include <typeindex>

namespace muon::event {

class Dispatcher : utils::NoCopy, utils::NoMove {
public:
    using EventDispatcher = eventpp::EventDispatcher<std::type_index, void(const void *)>;
    using Handle = EventDispatcher::Handle;

    template <typename Event>
    auto subscribe(std::function<void(const Event &)> listener) -> Handle {
        return dispatcher_.appendListener(typeid(Event), [listener](const void *event) {
            listener(*static_cast<const Event *>(event));
        });
    }

    template <typename Event>
    [[nodiscard]] auto unsubscribe(const Handle &handle) -> bool {
        return dispatcher_.removeListener(typeid(Event), handle);
    }

    template <typename Event>
    void dispatch(const Event &event) const {
        dispatcher_.dispatch(typeid(Event), &event);
    }

private:
    EventDispatcher dispatcher_;
};

} // namespace muon::event
