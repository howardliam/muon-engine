#include "muon/engine/log/logger.hpp"

namespace mu::log {
    static common::log::BasicLogger basicLogger;
    common::log::ILogger *globalLogger = &basicLogger;

    void setLogger(common::log::ILogger *logger) {
        globalLogger = logger;
    }
}
