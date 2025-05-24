#pragma once

#include <memory>
#include <spdlog/spdlog.h>

// idea borrowed from https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Log.h

namespace mu {

    class Log {
    public:
        static void init();

        static std::shared_ptr<spdlog::logger> &getCoreLogger();
        static std::shared_ptr<spdlog::logger> &getClientLogger();
        static std::shared_ptr<spdlog::logger> &getVulkanLogger();

    private:
        static std::shared_ptr<spdlog::logger> coreLogger;
        static std::shared_ptr<spdlog::logger> clientLogger;
        static std::shared_ptr<spdlog::logger> vulkanLogger;
    };

}

#ifdef MU_DEBUG_ENABLED

    #define MU_CORE_TRACE(...)      mu::Log::getCoreLogger()->trace(__VA_ARGS__)
    #define MU_CORE_DEBUG(...)      mu::Log::getCoreLogger()->debug(__VA_ARGS__)

    #define MU_TRACE(...)           mu::Log::getClientLogger()->trace(__VA_ARGS__)
    #define MU_DEBUG(...)           mu::Log::getClientLogger()->debug(__VA_ARGS__)

    #define MU_VK_TRACE(...)        mu::Log::getVulkanLogger()->trace(__VA_ARGS__)
    #define MU_VK_DEBUG(...)        mu::Log::getVulkanLogger()->debug(__VA_ARGS__)

#else

    #define MU_CORE_TRACE(...)
    #define MU_CORE_DEBUG(...)

    #define MU_TRACE(...)
    #define MU_DEBUG(...)

    #define MU_VK_TRACE(...)
    #define MU_VK_DEBUG(...)

#endif

#define MU_CORE_INFO(...)       mu::Log::getCoreLogger()->info(__VA_ARGS__)
#define MU_CORE_WARN(...)       mu::Log::getCoreLogger()->warn(__VA_ARGS__)
#define MU_CORE_ERROR(...)      mu::Log::getCoreLogger()->error(__VA_ARGS__)
#define MU_CORE_CRITICAL(...)   mu::Log::getCoreLogger()->critical(__VA_ARGS__)

#define MU_INFO(...)            mu::Log::getClientLogger()->info(__VA_ARGS__)
#define MU_WARN(...)            mu::Log::getClientLogger()->warn(__VA_ARGS__)
#define MU_ERROR(...)           mu::Log::getClientLogger()->error(__VA_ARGS__)
#define MU_CRITICAL(...)        mu::Log::getClientLogger()->critical(__VA_ARGS__)

#define MU_VK_INFO(...)         mu::Log::getVulkanLogger()->info(__VA_ARGS__)
#define MU_VK_WARN(...)         mu::Log::getVulkanLogger()->warn(__VA_ARGS__)
#define MU_VK_ERROR(...)        mu::Log::getVulkanLogger()->error(__VA_ARGS__)
#define MU_VK_CRITICAL(...)     mu::Log::getVulkanLogger()->critical(__VA_ARGS__)
