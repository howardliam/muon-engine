#pragma once

#include "muon/common/log/logger.hpp"

namespace mu::log {
    extern muon::common::log::ILogger *globalLogger;

    void setLogger(muon::common::log::ILogger *logger);

}
