#include "logger.hpp"

#include <spdlog/spdlog.h>

Logger::Logger() : muon::common::log::ILogger() {
    spdlog::set_level(spdlog::level::trace);
}
void Logger::traceImpl(const std::string_view message) {
    spdlog::trace(message);
}

void Logger::debugImpl(const std::string_view message) {
    spdlog::debug(message);
}

void Logger::infoImpl(const std::string_view message) {
    spdlog::info(message);
}

void Logger::warnImpl(const std::string_view message) {
    spdlog::warn(message);
}

void Logger::errorImpl(const std::string_view message) {
    spdlog::error(message);
}
