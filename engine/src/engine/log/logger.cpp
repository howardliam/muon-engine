#include "muon/engine/log/logger.hpp"

namespace mu::log {
    static muon::common::log::BasicLogger basicLogger;
    muon::common::log::ILogger *globalLogger = &basicLogger;

    void setLogger(muon::common::log::ILogger *logger) {
        globalLogger = logger;
    }
}
