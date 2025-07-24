#pragma once

#include "fmt/base.h"
#include "spdlog/logger.h"

#include <memory>

// idea borrowed from https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Log.h

namespace muon {

class Log {
public:
    static auto Init() -> void;

    static auto GetCoreLogger() -> std::shared_ptr<spdlog::logger> &;
    static auto GetClientLogger() -> std::shared_ptr<spdlog::logger> &;
    static auto GetVulkanLogger() -> std::shared_ptr<spdlog::logger> &;

    static auto SetLogLevel(spdlog::level::level_enum level) -> void;

private:
    static inline std::shared_ptr<spdlog::logger> s_coreLogger{nullptr};
    static inline std::shared_ptr<spdlog::logger> s_clientLogger{nullptr};
    static inline std::shared_ptr<spdlog::logger> s_vulkanLogger{nullptr};
};

namespace core {

template <typename... Args>
inline auto trace(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::GetCoreLogger()->trace(message, std::forward<Args>(args)...);
}

inline auto trace(const char *message) -> void { Log::GetCoreLogger()->trace(message); }

template <typename... Args>
inline auto debug(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::GetCoreLogger()->debug(message, std::forward<Args>(args)...);
}

inline auto debug(const char *message) -> void { Log::GetCoreLogger()->debug(message); }

template <typename... Args>
inline auto info(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::GetCoreLogger()->info(message, std::forward<Args>(args)...);
}

inline auto info(const char *message) -> void { Log::GetCoreLogger()->info(message); }

template <typename... Args>
inline auto warn(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::GetCoreLogger()->warn(message, std::forward<Args>(args)...);
}

inline auto warn(const char *message) -> void { Log::GetCoreLogger()->warn(message); }

template <typename... Args>
inline auto error(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::GetCoreLogger()->error(message, std::forward<Args>(args)...);
}

inline auto error(const char *message) -> void { Log::GetCoreLogger()->error(message); }

} // namespace core

namespace client {

template <typename... Args>
inline auto trace(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::GetClientLogger()->trace(message, std::forward<Args>(args)...);
}

inline auto trace(const char *message) -> void { Log::GetClientLogger()->trace(message); }

template <typename... Args>
inline auto debug(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::GetClientLogger()->debug(message, std::forward<Args>(args)...);
}

inline auto debug(const char *message) -> void { Log::GetClientLogger()->debug(message); }

template <typename... Args>
inline auto info(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::GetClientLogger()->info(message, std::forward<Args>(args)...);
}

inline auto info(const char *message) -> void { Log::GetClientLogger()->info(message); }

template <typename... Args>
inline auto warn(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::GetClientLogger()->warn(message, std::forward<Args>(args)...);
}

inline auto warn(const char *message) -> void { Log::GetClientLogger()->warn(message); }

template <typename... Args>
inline auto error(fmt::format_string<Args...> message, Args &&...args) -> void {
    Log::GetClientLogger()->error(message, std::forward<Args>(args)...);
}

inline auto error(const char *message) -> void { Log::GetClientLogger()->error(message); }

} // namespace client

} // namespace muon
