#include "muon/core/log.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog-inl.h>
#include <vector>

namespace muon {

auto Log::Init() -> void {
    std::vector<spdlog::sink_ptr> sinks;
    sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Muon.log", true));

    sinks[0]->set_pattern("%^[%T] %4n: %v%$");
    sinks[1]->set_pattern("[%T] [%l] %n: %v");

    s_coreLogger = std::make_shared<spdlog::logger>("MUON", sinks.begin(), sinks.end());
    spdlog::register_logger(s_coreLogger);
    s_coreLogger->set_level(spdlog::level::trace);
    s_coreLogger->flush_on(spdlog::level::trace);

    s_clientLogger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
    spdlog::register_logger(s_clientLogger);
    s_clientLogger->set_level(spdlog::level::trace);
    s_clientLogger->flush_on(spdlog::level::trace);

    s_vulkanLogger = std::make_shared<spdlog::logger>("VULK", sinks.begin(), sinks.end());
    spdlog::register_logger(s_vulkanLogger);
    s_vulkanLogger->set_level(spdlog::level::trace);
    s_vulkanLogger->flush_on(spdlog::level::trace);
}

auto Log::GetCoreLogger() -> std::shared_ptr<spdlog::logger> & { return s_coreLogger; }
auto Log::GetClientLogger() -> std::shared_ptr<spdlog::logger> & { return s_clientLogger; }
auto Log::GetVulkanLogger() -> std::shared_ptr<spdlog::logger> & { return s_vulkanLogger; }

} // namespace muon
