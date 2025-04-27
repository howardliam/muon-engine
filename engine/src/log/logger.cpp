#include "muon/log/logger.hpp"

#include <print>

namespace muon::log {
    static BasicLogger basicLogger;
    ILogger *globalLogger = &basicLogger;

    void setLogger(ILogger *logger) {
        globalLogger = logger;
    }

    BasicLogger::BasicLogger() : ILogger() {}

    void BasicLogger::traceImpl(const std::string &message) {
        std::println("[TRACE]: {}", message);
    }

    void BasicLogger::debugImpl(const std::string &message) {
        std::println("[DEBUG]: {}", message);
    }

    void BasicLogger::infoImpl(const std::string &message) {
        std::println("[INFO ]: {}", message);
    }

    void BasicLogger::warnImpl(const std::string &message) {
        std::println("[WARN ]: {}", message);
    }

    void BasicLogger::errorImpl(const std::string &message) {
        std::println("[ERROR]: {}", message);
    }
}
