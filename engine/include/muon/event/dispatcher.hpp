#pragma once

#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <eventpp/eventdispatcher.h>
#include <typeindex>

namespace muon::event {


    class Dispatcher : NoCopy, NoMove {
    private:
        using EventDispatcher = eventpp::EventDispatcher<std::type_index, void(const void *)>;

    public:
        using Handle = EventDispatcher::Handle;

    public:
        Dispatcher() = default;
        ~Dispatcher() = default;

        template<typename Event>
        [[nodiscard]] Handle Subscribe(std::function<void(const Event &)> listener) {
            return m_dispatcher.appendListener(typeid(Event), [listener](const void *event) {
                listener(*static_cast<const Event *>(event));
            });
        }

        template<typename Event>
        [[nodiscard]] bool Unsubscribe(const Handle handle) {
            return m_dispatcher.removeListener(typeid(Event), handle);
        }

        template<typename Event>
        void Dispatch(const Event &event) const {
            m_dispatcher.dispatch(typeid(Event), &event);
        }

    private:
        EventDispatcher m_dispatcher;
    };

}
