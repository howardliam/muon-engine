#pragma once

#include "muon/utils/no_copy.hpp"
#include "muon/utils/no_move.hpp"

#include <eventpp/eventdispatcher.h>
#include <typeindex>

namespace muon::event {

class dispatcher : utils::no_copy, utils::no_move {
private:
    using event_dispatcher = eventpp::EventDispatcher<std::type_index, void(const void *)>;

public:
    using handle = event_dispatcher::Handle;

public:
    template <typename Event>
    auto subscribe(std::function<void(const Event &)> listener) -> handle {
        return dispatcher_.appendListener(typeid(Event), [listener](const void *event) {
            listener(*static_cast<const Event *>(event));
        });
    }

    template <typename Event>
    [[nodiscard]] auto unsubscribe(const handle &handle) -> bool {
        return dispatcher_.removeListener(typeid(Event), handle);
    }

    template <typename Event>
    void dispatch(const Event &event) const {
        dispatcher_.dispatch(typeid(Event), &event);
    }

private:
    event_dispatcher dispatcher_;
};

} // namespace muon::event
