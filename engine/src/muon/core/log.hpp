#pragma once

#include <memory>
#include <spdlog/spdlog.h>

// idea borrowed from https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Log.h

namespace muon {

enum class Logger {
    Core,
    Client,
};

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
constexpr inline auto trace(std::string_view message, Args &&...args) -> void {
    Log::GetCoreLogger()->trace(message, args...);
}

template <typename... Args>
constexpr inline auto debug(std::string_view message, Args &&...args) -> void {
    Log::GetCoreLogger()->debug(message, args...);
}

template <typename... Args>
constexpr inline auto info(std::string_view message, Args &&...args) -> void {
    Log::GetCoreLogger()->info(message, args...);
}

template <typename... Args>
constexpr inline auto warn(std::string_view message, Args &&...args) -> void {
    Log::GetCoreLogger()->warn(message, args...);
}

template <typename... Args>
constexpr inline auto error(std::string_view message, Args &&...args) -> void {
    Log::GetCoreLogger()->error(message, args...);
}

} // namespace core

namespace client {

template <typename... Args>
constexpr auto trace(std::string_view message, Args &&...args) -> void {
    Log::GetClientLogger()->trace(message, args...);
}

template <typename... Args>
constexpr auto debug(std::string_view message, Args &&...args) -> void {
    Log::GetClientLogger()->debug(message, args...);
}

template <typename... Args>
constexpr auto info(std::string_view message, Args &&...args) -> void {
    Log::GetClientLogger()->info(message, args...);
}

template <typename... Args>
constexpr auto warn(std::string_view message, Args &&...args) -> void {
    Log::GetClientLogger()->warn(message, args...);
}

template <typename... Args>
constexpr auto error(std::string_view message, Args &&...args) -> void {
    Log::GetClientLogger()->error(message, args...);
}

} // namespace client

} // namespace muon

#ifdef MU_DEBUG_ENABLED

#define MU_CORE_TRACE(...) muon::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define MU_CORE_DEBUG(...) muon::Log::GetCoreLogger()->debug(__VA_ARGS__)

#define MU_TRACE(...) muon::Log::GetClientLogger()->trace(__VA_ARGS__)
#define MU_DEBUG(...) muon::Log::GetClientLogger()->debug(__VA_ARGS__)

#define MU_VK_TRACE(...) muon::Log::GetVulkanLogger()->trace(__VA_ARGS__)
#define MU_VK_DEBUG(...) muon::Log::GetVulkanLogger()->debug(__VA_ARGS__)

#else

#define MU_CORE_TRACE(...)
#define MU_CORE_DEBUG(...)

#define MU_TRACE(...)
#define MU_DEBUG(...)

#define MU_VK_TRACE(...)
#define MU_VK_DEBUG(...)

#endif

#define MU_CORE_INFO(...) muon::Log::GetCoreLogger()->info(__VA_ARGS__)
#define MU_CORE_WARN(...) muon::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define MU_CORE_ERROR(...) muon::Log::GetCoreLogger()->error(__VA_ARGS__)
#define MU_CORE_CRITICAL(...) muon::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define MU_INFO(...) muon::Log::GetClientLogger()->info(__VA_ARGS__)
#define MU_WARN(...) muon::Log::GetClientLogger()->warn(__VA_ARGS__)
#define MU_ERROR(...) muon::Log::GetClientLogger()->error(__VA_ARGS__)
#define MU_CRITICAL(...) muon::Log::GetClientLogger()->critical(__VA_ARGS__)

#define MU_VK_INFO(...) muon::Log::GetVulkanLogger()->info(__VA_ARGS__)
#define MU_VK_WARN(...) muon::Log::GetVulkanLogger()->warn(__VA_ARGS__)
#define MU_VK_ERROR(...) muon::Log::GetVulkanLogger()->error(__VA_ARGS__)
#define MU_VK_CRITICAL(...) muon::Log::GetVulkanLogger()->critical(__VA_ARGS__)
