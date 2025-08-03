#include "muon/core/log.hpp"

#include "fmt/format.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <mutex>
#include <vector>

namespace muon {

namespace log {

std::once_flag g_loggerInit;

namespace internal {

std::shared_ptr<spdlog::logger> s_coreLogger{nullptr};
std::shared_ptr<spdlog::logger> s_clientLogger{nullptr};

} // namespace internal

void init() {
    auto initSpdlog = []() {
        std::vector<spdlog::sink_ptr> sinks;
        sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Muon.log", true));

        sinks[0]->set_pattern("%T%z [%4n] [%^%5l%$]: %v");
        sinks[1]->set_pattern("%T%z [%4n] [%5l]: %v");

        internal::s_coreLogger = std::make_shared<spdlog::logger>("MUON", sinks.begin(), sinks.end());
        spdlog::register_logger(internal::s_coreLogger);
        internal::s_coreLogger->set_level(spdlog::level::trace);
        internal::s_coreLogger->flush_on(spdlog::level::trace);

        internal::s_clientLogger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
        spdlog::register_logger(internal::s_clientLogger);
        internal::s_clientLogger->set_level(spdlog::level::trace);
        internal::s_clientLogger->flush_on(spdlog::level::trace);
    };

    std::call_once(g_loggerInit, initSpdlog);
}

} // namespace log

} // namespace muon
