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
inline void trace(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (k_debugEnabled) {
    }
    log::internal::s_coreLogger->trace(message, std::forward<Args>(args)...);
}
inline void trace(const char *message) {
    if constexpr (k_debugEnabled) {
    }
    log::internal::s_coreLogger->trace(message);
}

template <typename... Args>
inline void debug(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (k_debugEnabled) {
    }
    log::internal::s_coreLogger->debug(message, std::forward<Args>(args)...);
}
inline void debug(const char *message) {
    if constexpr (k_debugEnabled) {
    }
    log::internal::s_coreLogger->debug(message);
}

template <typename... Args>
inline void info(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_coreLogger->info(message, std::forward<Args>(args)...);
}
inline void info(const char *message) { log::internal::s_coreLogger->info(message); }

template <typename... Args>
inline void warn(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_coreLogger->warn(message, std::forward<Args>(args)...);
}
inline void warn(const char *message) { log::internal::s_coreLogger->warn(message); }

template <typename... Args>
inline void error(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_coreLogger->error(message, std::forward<Args>(args)...);
}
inline void error(const char *message) { log::internal::s_coreLogger->error(message); }

template <typename... Args>
inline void critical(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_coreLogger->critical(message, std::forward<Args>(args)...);
}
inline void critical(const char *message) { log::internal::s_coreLogger->critical(message); }

} // namespace core

namespace client {

template <typename... Args>
inline void trace(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (k_debugEnabled) {
        log::internal::s_clientLogger->trace(message, std::forward<Args>(args)...);
    }
}
inline void trace(const char *message) {
    if constexpr (k_debugEnabled) {
        log::internal::s_clientLogger->trace(message);
    }
}

template <typename... Args>
inline void debug(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (k_debugEnabled) {
        log::internal::s_clientLogger->debug(message, std::forward<Args>(args)...);
    }
}
inline void debug(const char *message) {
    if constexpr (k_debugEnabled) {
        log::internal::s_clientLogger->debug(message);
    }
}

template <typename... Args>
inline void info(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_clientLogger->info(message, std::forward<Args>(args)...);
}
inline void info(const char *message) { log::internal::s_clientLogger->info(message); }

template <typename... Args>
inline void warn(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_clientLogger->warn(message, std::forward<Args>(args)...);
}
inline void warn(const char *message) { log::internal::s_clientLogger->warn(message); }

template <typename... Args>
inline void error(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_clientLogger->error(message, std::forward<Args>(args)...);
}
inline void error(const char *message) { log::internal::s_clientLogger->error(message); }

template <typename... Args>
inline void critical(fmt::format_string<Args...> message, Args &&...args) {
    log::internal::s_clientLogger->critical(message, std::forward<Args>(args)...);
}
inline void critical(const char *message) { log::internal::s_clientLogger->critical(message); }

} // namespace client

} // namespace muon
