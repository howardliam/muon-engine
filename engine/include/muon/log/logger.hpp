#pragma once

#include "muon/common/log/logger.hpp"

namespace muon::log {
    extern common::log::ILogger *globalLogger;

    void setLogger(common::log::ILogger *logger);

}
