#pragma once

#include <memory>
#include <spdlog/spdlog.h>

// idea borrowed from https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Log.h

namespace muon {

    class Log {
    public:
        static void Init();

        static std::shared_ptr<spdlog::logger> &GetCoreLogger();
        static std::shared_ptr<spdlog::logger> &GetClientLogger();
        static std::shared_ptr<spdlog::logger> &GetVulkanLogger();

    private:
        static inline std::shared_ptr<spdlog::logger> s_coreLogger = nullptr;
        static inline std::shared_ptr<spdlog::logger> s_clientLogger = nullptr;
        static inline std::shared_ptr<spdlog::logger> s_vulkanLogger = nullptr;
    };

}

#ifdef MU_DEBUG_ENABLED

    #define MU_CORE_TRACE(...)      muon::Log::GetCoreLogger()->trace(__VA_ARGS__)
    #define MU_CORE_DEBUG(...)      muon::Log::GetCoreLogger()->debug(__VA_ARGS__)

    #define MU_TRACE(...)           muon::Log::GetClientLogger()->trace(__VA_ARGS__)
    #define MU_DEBUG(...)           muon::Log::GetClientLogger()->debug(__VA_ARGS__)

    #define MU_VK_TRACE(...)        muon::Log::GetVulkanLogger()->trace(__VA_ARGS__)
    #define MU_VK_DEBUG(...)        muon::Log::GetVulkanLogger()->debug(__VA_ARGS__)

#else

    #define MU_CORE_TRACE(...)
    #define MU_CORE_DEBUG(...)

    #define MU_TRACE(...)
    #define MU_DEBUG(...)

    #define MU_VK_TRACE(...)
    #define MU_VK_DEBUG(...)

#endif

#define MU_CORE_INFO(...)       muon::Log::GetCoreLogger()->info(__VA_ARGS__)
#define MU_CORE_WARN(...)       muon::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define MU_CORE_ERROR(...)      muon::Log::GetCoreLogger()->error(__VA_ARGS__)
#define MU_CORE_CRITICAL(...)   muon::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define MU_INFO(...)            muon::Log::GetClientLogger()->info(__VA_ARGS__)
#define MU_WARN(...)            muon::Log::GetClientLogger()->warn(__VA_ARGS__)
#define MU_ERROR(...)           muon::Log::GetClientLogger()->error(__VA_ARGS__)
#define MU_CRITICAL(...)        muon::Log::GetClientLogger()->critical(__VA_ARGS__)

#define MU_VK_INFO(...)         muon::Log::GetVulkanLogger()->info(__VA_ARGS__)
#define MU_VK_WARN(...)         muon::Log::GetVulkanLogger()->warn(__VA_ARGS__)
#define MU_VK_ERROR(...)        muon::Log::GetVulkanLogger()->error(__VA_ARGS__)
#define MU_VK_CRITICAL(...)     muon::Log::GetVulkanLogger()->critical(__VA_ARGS__)
