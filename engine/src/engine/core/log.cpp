#include "muon/engine/core/log.hpp"

#include <spdlog/spdlog-inl.h>
#include <vector>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace mu {

    std::shared_ptr<spdlog::logger> Log::coreLogger;
    std::shared_ptr<spdlog::logger> Log::clientLogger;
    std::shared_ptr<spdlog::logger> Log::vulkanLogger;


    void Log::init() {
        std::vector<spdlog::sink_ptr> sinks;
        sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Muon.log", true));

        sinks[0]->set_pattern("%^[%T] %n: %v%$");
        sinks[1]->set_pattern("[%T] [%l] %n: %v");

        coreLogger = std::make_shared<spdlog::logger>("MUON", sinks.begin(), sinks.end());
        spdlog::register_logger(coreLogger);
        coreLogger->set_level(spdlog::level::trace);
        coreLogger->flush_on(spdlog::level::trace);

        clientLogger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
        spdlog::register_logger(clientLogger);
        clientLogger->set_level(spdlog::level::trace);
        clientLogger->flush_on(spdlog::level::trace);

        vulkanLogger = std::make_shared<spdlog::logger>("VK", sinks.begin(), sinks.end());
        spdlog::register_logger(vulkanLogger);
        vulkanLogger->set_level(spdlog::level::trace);
        vulkanLogger->flush_on(spdlog::level::trace);
    }

    std::shared_ptr<spdlog::logger> &Log::getCoreLogger() {
        return coreLogger;
    }

    std::shared_ptr<spdlog::logger> &Log::getClientLogger() {
        return clientLogger;
    }

        std::shared_ptr<spdlog::logger> &Log::getVulkanLogger() {
            return vulkanLogger;
        }

}
