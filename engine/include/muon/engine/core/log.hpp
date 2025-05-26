#pragma once

#include <memory>
#include <spdlog/spdlog.h>

// idea borrowed from https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Log.h

namespace muon {

    class Log {
    public:
        static void init();

        static std::shared_ptr<spdlog::logger> &coreLogger();
        static std::shared_ptr<spdlog::logger> &clientLogger();
        static std::shared_ptr<spdlog::logger> &vulkanLogger();

    private:
        static std::shared_ptr<spdlog::logger> s_coreLogger;
        static std::shared_ptr<spdlog::logger> s_clientLogger;
        static std::shared_ptr<spdlog::logger> s_vulkanLogger;
    };

}

#ifdef MU_DEBUG_ENABLED

    #define MU_CORE_TRACE(...)      muon::Log::coreLogger()->trace(__VA_ARGS__)
    #define MU_CORE_DEBUG(...)      muon::Log::coreLogger()->debug(__VA_ARGS__)

    #define MU_TRACE(...)           muon::Log::clientLogger()->trace(__VA_ARGS__)
    #define MU_DEBUG(...)           muon::Log::clientLogger()->debug(__VA_ARGS__)

    #define MU_VK_TRACE(...)        muon::Log::vulkanLogger()->trace(__VA_ARGS__)
    #define MU_VK_DEBUG(...)        muon::Log::vulkanLogger()->debug(__VA_ARGS__)

#else

    #define MU_CORE_TRACE(...)
    #define MU_CORE_DEBUG(...)

    #define MU_TRACE(...)
    #define MU_DEBUG(...)

    #define MU_VK_TRACE(...)
    #define MU_VK_DEBUG(...)

#endif

#define MU_CORE_INFO(...)       muon::Log::coreLogger()->info(__VA_ARGS__)
#define MU_CORE_WARN(...)       muon::Log::coreLogger()->warn(__VA_ARGS__)
#define MU_CORE_ERROR(...)      muon::Log::coreLogger()->error(__VA_ARGS__)
#define MU_CORE_CRITICAL(...)   muon::Log::coreLogger()->critical(__VA_ARGS__)

#define MU_INFO(...)            muon::Log::clientLogger()->info(__VA_ARGS__)
#define MU_WARN(...)            muon::Log::clientLogger()->warn(__VA_ARGS__)
#define MU_ERROR(...)           muon::Log::clientLogger()->error(__VA_ARGS__)
#define MU_CRITICAL(...)        muon::Log::clientLogger()->critical(__VA_ARGS__)

#define MU_VK_INFO(...)         muon::Log::vulkanLogger()->info(__VA_ARGS__)
#define MU_VK_WARN(...)         muon::Log::vulkanLogger()->warn(__VA_ARGS__)
#define MU_VK_ERROR(...)        muon::Log::vulkanLogger()->error(__VA_ARGS__)
#define MU_VK_CRITICAL(...)     muon::Log::vulkanLogger()->critical(__VA_ARGS__)
