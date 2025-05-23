#pragma once

#include "muon/common/log/logger.hpp"

namespace mu::log {
    extern common::log::ILogger *globalLogger;

    void setLogger(common::log::ILogger *logger);

}
