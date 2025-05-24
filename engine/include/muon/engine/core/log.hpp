#pragma once

#include <memory>
#include <spdlog/spdlog.h>

// idea borrowed from https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Log.h

namespace muon {

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

    #define MU_CORE_TRACE(...)      muon::Log::getCoreLogger()->trace(__VA_ARGS__)
    #define MU_CORE_DEBUG(...)      muon::Log::getCoreLogger()->debug(__VA_ARGS__)

    #define MU_TRACE(...)           muon::Log::getClientLogger()->trace(__VA_ARGS__)
    #define MU_DEBUG(...)           muon::Log::getClientLogger()->debug(__VA_ARGS__)

    #define MU_VK_TRACE(...)        muon::Log::getVulkanLogger()->trace(__VA_ARGS__)
    #define MU_VK_DEBUG(...)        muon::Log::getVulkanLogger()->debug(__VA_ARGS__)

#else

    #define MU_CORE_TRACE(...)
    #define MU_CORE_DEBUG(...)

    #define MU_TRACE(...)
    #define MU_DEBUG(...)

    #define MU_VK_TRACE(...)
    #define MU_VK_DEBUG(...)

#endif

#define MU_CORE_INFO(...)       muon::Log::getCoreLogger()->info(__VA_ARGS__)
#define MU_CORE_WARN(...)       muon::Log::getCoreLogger()->warn(__VA_ARGS__)
#define MU_CORE_ERROR(...)      muon::Log::getCoreLogger()->error(__VA_ARGS__)
#define MU_CORE_CRITICAL(...)   muon::Log::getCoreLogger()->critical(__VA_ARGS__)

#define MU_INFO(...)            muon::Log::getClientLogger()->info(__VA_ARGS__)
#define MU_WARN(...)            muon::Log::getClientLogger()->warn(__VA_ARGS__)
#define MU_ERROR(...)           muon::Log::getClientLogger()->error(__VA_ARGS__)
#define MU_CRITICAL(...)        muon::Log::getClientLogger()->critical(__VA_ARGS__)

#define MU_VK_INFO(...)         muon::Log::getVulkanLogger()->info(__VA_ARGS__)
#define MU_VK_WARN(...)         muon::Log::getVulkanLogger()->warn(__VA_ARGS__)
#define MU_VK_ERROR(...)        muon::Log::getVulkanLogger()->error(__VA_ARGS__)
#define MU_VK_CRITICAL(...)     muon::Log::getVulkanLogger()->critical(__VA_ARGS__)
