#pragma once

#include "fmt/base.h"
#include "spdlog/logger.h"

#include <memory>

namespace muon {

class Log {
public:
    static auto init() -> void;

    static auto getCoreLogger() -> std::shared_ptr<spdlog::logger> &;
    static auto getClientLogger() -> std::shared_ptr<spdlog::logger> &;
    static auto getVulkanLogger() -> std::shared_ptr<spdlog::logger> &;

    static auto SetLogLevel(spdlog::level::level_enum level) -> void;

private:
    static inline std::shared_ptr<spdlog::logger> s_coreLogger{nullptr};
    static inline std::shared_ptr<spdlog::logger> s_clientLogger{nullptr};
    static inline std::shared_ptr<spdlog::logger> s_vulkanLogger{nullptr};
};

namespace core {

template <typename... Args>
inline auto trace(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::getCoreLogger()->trace(message, std::forward<Args>(args)...);
}

inline auto trace(const char *message) -> void { Log::getCoreLogger()->trace(message); }

template <typename... Args>
inline auto debug(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::getCoreLogger()->debug(message, std::forward<Args>(args)...);
}

inline auto debug(const char *message) -> void { Log::getCoreLogger()->debug(message); }

template <typename... Args>
inline auto info(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::getCoreLogger()->info(message, std::forward<Args>(args)...);
}

inline auto info(const char *message) -> void { Log::getCoreLogger()->info(message); }

template <typename... Args>
inline auto warn(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::getCoreLogger()->warn(message, std::forward<Args>(args)...);
}

inline auto warn(const char *message) -> void { Log::getCoreLogger()->warn(message); }

template <typename... Args>
inline auto error(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::getCoreLogger()->error(message, std::forward<Args>(args)...);
}

inline auto error(const char *message) -> void { Log::getCoreLogger()->error(message); }

} // namespace core

namespace client {

template <typename... Args>
inline auto trace(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::getClientLogger()->trace(message, std::forward<Args>(args)...);
}

inline auto trace(const char *message) -> void { Log::getClientLogger()->trace(message); }

template <typename... Args>
inline auto debug(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::getClientLogger()->debug(message, std::forward<Args>(args)...);
}

inline auto debug(const char *message) -> void { Log::getClientLogger()->debug(message); }

template <typename... Args>
inline auto info(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::getClientLogger()->info(message, std::forward<Args>(args)...);
}

inline auto info(const char *message) -> void { Log::getClientLogger()->info(message); }

template <typename... Args>
inline auto warn(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::getClientLogger()->warn(message, std::forward<Args>(args)...);
}

inline auto warn(const char *message) -> void { Log::getClientLogger()->warn(message); }

template <typename... Args>
inline auto error(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::getClientLogger()->error(message, std::forward<Args>(args)...);
}

inline auto error(const char *message) -> void { Log::getClientLogger()->error(message); }

} // namespace client

} // namespace muon
