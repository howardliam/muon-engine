#include "muon/common/log/logger.hpp"

#include <print>

namespace mu::common::log {

    void BasicLogger::traceImpl(const std::string_view message) {
        std::println("[TRACE]: {}", message);
    }

    void BasicLogger::debugImpl(const std::string_view message) {
        std::println("[DEBUG]: {}", message);
    }

    void BasicLogger::infoImpl(const std::string_view message) {
        std::println("[INFO ]: {}", message);
    }

    void BasicLogger::warnImpl(const std::string_view message) {
        std::println("[WARN ]: {}", message);
    }

    void BasicLogger::errorImpl(const std::string_view message) {
        std::println("[ERROR]: {}", message);
    }

}
