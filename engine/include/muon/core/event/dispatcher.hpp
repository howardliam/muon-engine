#pragma once

#include <eventpp/eventdispatcher.h>
#include "muon/core/event/event.hpp"

namespace muon {

    using EventDispatcher = eventpp::EventDispatcher<EventType, void (const Event &), EventPolicies>;

}
