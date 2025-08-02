#pragma once

#include "fmt/base.h"
#include "muon/core/debug.hpp"
#include "spdlog/logger.h"

#include <memory>

namespace muon {

class Log {
public:
    static void init();

    static auto getCoreLogger() -> std::shared_ptr<spdlog::logger> &;
    static auto getClientLogger() -> std::shared_ptr<spdlog::logger> &;

    static void setLogLevel(spdlog::level::level_enum level);

private:
    static inline std::shared_ptr<spdlog::logger> s_coreLogger{nullptr};
    static inline std::shared_ptr<spdlog::logger> s_clientLogger{nullptr};
};

namespace core {

template <typename... Args>
inline void trace(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (k_debugEnabled) {
        Log::getCoreLogger()->trace(message, std::forward<Args>(args)...);
    }
}
inline void trace(const char *message) {
    if constexpr (k_debugEnabled) {
        Log::getCoreLogger()->trace(message);
    }
}

template <typename... Args>
inline void debug(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (k_debugEnabled) {
        Log::getCoreLogger()->debug(message, std::forward<Args>(args)...);
    }
}
inline void debug(const char *message) {
    if constexpr (k_debugEnabled) {
        Log::getCoreLogger()->debug(message);
    }
}

template <typename... Args>
inline void info(fmt::format_string<Args...> message, Args &&...args) {
    Log::getCoreLogger()->info(message, std::forward<Args>(args)...);
}
inline void info(const char *message) { Log::getCoreLogger()->info(message); }

template <typename... Args>
inline void warn(fmt::format_string<Args...> message, Args &&...args) {
    Log::getCoreLogger()->warn(message, std::forward<Args>(args)...);
}
inline void warn(const char *message) { Log::getCoreLogger()->warn(message); }

template <typename... Args>
inline void error(fmt::format_string<Args...> message, Args &&...args) {
    Log::getCoreLogger()->error(message, std::forward<Args>(args)...);
}
inline void error(const char *message) { Log::getCoreLogger()->error(message); }

} // namespace core

namespace client {

template <typename... Args>
inline void trace(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (k_debugEnabled) {
        Log::getClientLogger()->trace(message, std::forward<Args>(args)...);
    }
}
inline void trace(const char *message) {
    if constexpr (k_debugEnabled) {
        Log::getClientLogger()->trace(message);
    }
}

template <typename... Args>
inline void debug(fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (k_debugEnabled) {
        Log::getClientLogger()->debug(message, std::forward<Args>(args)...);
    }
}
inline void debug(const char *message) {
    if constexpr (k_debugEnabled) {
        Log::getClientLogger()->debug(message);
    }
}

template <typename... Args>
inline void info(fmt::format_string<Args...> message, Args &&...args) {
    Log::getClientLogger()->info(message, std::forward<Args>(args)...);
}
inline void info(const char *message) { Log::getClientLogger()->info(message); }

template <typename... Args>
inline void warn(fmt::format_string<Args...> message, Args &&...args) {
    Log::getClientLogger()->warn(message, std::forward<Args>(args)...);
}
inline void warn(const char *message) { Log::getClientLogger()->warn(message); }

template <typename... Args>
inline void error(fmt::format_string<Args...> message, Args &&...args) {
    Log::getClientLogger()->error(message, std::forward<Args>(args)...);
}
inline void error(const char *message) { Log::getClientLogger()->error(message); }

} // namespace client

} // namespace muon
