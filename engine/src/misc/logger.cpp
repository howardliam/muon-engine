#include "muon/misc/logger.hpp"

#include <print>

namespace muon::misc {
    BasicLogger::BasicLogger() : ILogger() {}

    void BasicLogger::traceImpl(std::string message) {
        std::println("[TRACE]: {}", message);
    }

    void BasicLogger::debugImpl(std::string message) {
        std::println("[DEBUG]: {}", message);
    }

    void BasicLogger::infoImpl(std::string message) {
        std::println("[INFO ]: {}", message);
    }

    void BasicLogger::warnImpl(std::string message) {
        std::println("[WARN ]: {}", message);
    }

    void BasicLogger::errorImpl(std::string message) {
        std::println("[ERROR]: {}", message);
    }
}
