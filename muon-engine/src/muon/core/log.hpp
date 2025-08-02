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

    static auto setLogLevel(spdlog::level::level_enum level) -> void;

private:
    static inline std::shared_ptr<spdlog::logger> s_coreLogger{nullptr};
    static inline std::shared_ptr<spdlog::logger> s_clientLogger{nullptr};
};

namespace core {

#ifdef MU_DEBUG

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

#else

template <typename... Args>
inline auto trace(fmt::format_string<Args...> message, Args &&...args) -> void {}
inline auto trace(const char *message) -> void {}

template <typename... Args>
inline auto debug(fmt::format_string<Args...> message, Args &&...args) -> void {}
inline auto debug(const char *message) -> void {}

#endif

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

#ifdef MU_DEBUG

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

#else

template <typename... Args>
inline auto trace(fmt::format_string<Args...> message, Args &&...args) -> void {}
inline auto trace(const char *message) -> void {}

template <typename... Args>
inline auto debug(fmt::format_string<Args...> message, Args &&...args) -> void {}
inline auto debug(const char *message) -> void {}

#endif

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
