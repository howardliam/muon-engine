#pragma once

#include "fmt/base.h"
#include "muon/core/debug.hpp"
#include "spdlog/logger.h"

#include <memory>

namespace muon {

namespace log {

namespace internal {

extern std::shared_ptr<spdlog::logger> core_logger;
extern std::shared_ptr<spdlog::logger> client_logger;

} // namespace internal

void init();

} // namespace log

namespace core {

template <typename... Args>
void trace(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (debug_enabled) {
        log::internal::core_logger->trace(message, std::forward<Args>(args)...);
    }
}
void trace(const char *message);

template <typename... Args>
void debug(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (debug_enabled) {
        log::internal::core_logger->debug(message, std::forward<Args>(args)...);
    }
}
void debug(const char *message);

template <typename... Args>
void info(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::core_logger->info(message, std::forward<Args>(args)...);
}
void info(const char *message);

template <typename... Args>
void warn(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::core_logger->warn(message, std::forward<Args>(args)...);
}
void warn(const char *message);

template <typename... Args>
void error(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::core_logger->error(message, std::forward<Args>(args)...);
}
void error(const char *message);

template <typename... Args>
void critical(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::core_logger->critical(message, std::forward<Args>(args)...);
}
void critical(const char *message);

} // namespace core

namespace client {

template <typename... Args>
void trace(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (debug_enabled) {
        log::internal::client_logger->trace(message, std::forward<Args>(args)...);
    }
}
void trace(const char *message);

template <typename... Args>
void debug(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (debug_enabled) {
        log::internal::client_logger->debug(message, std::forward<Args>(args)...);
    }
}
void debug(const char *message);

template <typename... Args>
void info(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::client_logger->info(message, std::forward<Args>(args)...);
}
void info(const char *message);

template <typename... Args>
void warn(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::client_logger->warn(message, std::forward<Args>(args)...);
}
void warn(const char *message);

template <typename... Args>
void error(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::client_logger->error(message, std::forward<Args>(args)...);
}
void error(const char *message);

template <typename... Args>
void critical(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::client_logger->critical(message, std::forward<Args>(args)...);
}
void critical(const char *message);

} // namespace client

} // namespace muon
