#pragma once

#include <eventpp/eventdispatcher.h>
#include <typeindex>

namespace muon::event {

    class Dispatcher {
    public:
        Dispatcher() = default;
        ~Dispatcher() = default;

        template<typename T>
        void Subscribe(std::function<void(const T &)> listener) {
            m_dispatcher.appendListener(typeid(T), [listener](const void *event) {
                listener(*static_cast<const T *>(event));
            });
        }

        template<typename T>
        void Dispatch(const T &event) const {
            m_dispatcher.dispatch(typeid(T), &event);
        }

    private:
        eventpp::EventDispatcher<std::type_index, void(const void *)> m_dispatcher;
    };

}
