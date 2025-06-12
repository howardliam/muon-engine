#pragma once

#include "muon/event/event.hpp"
#include <eventpp/eventdispatcher.h>

namespace muon::event {

    using EventDispatcher = eventpp::EventDispatcher<EventType, void (const Event &), EventPolicies>;

}
