#pragma once

#include "fmt/base.h"
#include "muon/core/debug.hpp"
#include "spdlog/logger.h"

#include <memory>

namespace muon {

namespace log {

namespace internal {

extern std::shared_ptr<spdlog::logger> s_coreLogger;
extern std::shared_ptr<spdlog::logger> s_clientLogger;

} // namespace internal

void init();

} // namespace log

namespace core {

template <typename... Args>
void trace(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (k_debugEnabled) {
        log::internal::s_coreLogger->trace(message, std::forward<Args>(args)...);
    }
}
void trace(const char *message);

template <typename... Args>
void debug(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (k_debugEnabled) {
        log::internal::s_coreLogger->debug(message, std::forward<Args>(args)...);
    }
}
void debug(const char *message);

template <typename... Args>
void info(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_coreLogger->info(message, std::forward<Args>(args)...);
}
void info(const char *message);

template <typename... Args>
void warn(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_coreLogger->warn(message, std::forward<Args>(args)...);
}
void warn(const char *message);

template <typename... Args>
void error(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_coreLogger->error(message, std::forward<Args>(args)...);
}
void error(const char *message);

template <typename... Args>
void critical(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_coreLogger->critical(message, std::forward<Args>(args)...);
}
void critical(const char *message);

} // namespace core

namespace client {

template <typename... Args>
void trace(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (k_debugEnabled) {
        log::internal::s_clientLogger->trace(message, std::forward<Args>(args)...);
    }
}
void trace(const char *message);

template <typename... Args>
void debug(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (k_debugEnabled) {
        log::internal::s_clientLogger->debug(message, std::forward<Args>(args)...);
    }
}
void debug(const char *message);

template <typename... Args>
void info(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_clientLogger->info(message, std::forward<Args>(args)...);
}
void info(const char *message);

template <typename... Args>
void warn(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_clientLogger->warn(message, std::forward<Args>(args)...);
}
void warn(const char *message);

template <typename... Args>
void error(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_clientLogger->error(message, std::forward<Args>(args)...);
}
void error(const char *message);

template <typename... Args>
void critical(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_clientLogger->critical(message, std::forward<Args>(args)...);
}
void critical(const char *message);

} // namespace client

} // namespace muon
