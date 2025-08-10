#include "muon/core/log.hpp"

#include "fmt/format.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

#include <mutex>
#include <vector>

namespace muon {

namespace log {

std::once_flag logger_init;

namespace internal {

std::shared_ptr<spdlog::logger> core_logger{nullptr};
std::shared_ptr<spdlog::logger> client_logger{nullptr};

} // namespace internal

void init() {
    auto init_spdlog = []() {
        std::vector<spdlog::sink_ptr> sinks;
        sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Muon.log", true));

        sinks[0]->set_pattern("%T%z [%4n] [%^%5l%$]: %v");
        sinks[1]->set_pattern("%T%z [%4n] [%5l]: %v");

        internal::core_logger = std::make_shared<spdlog::logger>("MUON", sinks.begin(), sinks.end());
        spdlog::register_logger(internal::core_logger);
        internal::core_logger->set_level(spdlog::level::trace);
        internal::core_logger->flush_on(spdlog::level::trace);

        internal::client_logger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
        spdlog::register_logger(internal::client_logger);
        internal::client_logger->set_level(spdlog::level::trace);
        internal::client_logger->flush_on(spdlog::level::trace);

        if constexpr (debug_enabled) {
            spdlog::set_level(spdlog::level::trace);
        } else {
            spdlog::set_level(spdlog::level::info);
        }
    };

    std::call_once(logger_init, init_spdlog);
}

} // namespace log

namespace core {

void trace(const char *message) {
    if constexpr (debug_enabled) {
        log::internal::core_logger->trace(message);
    }
}

void debug(const char *message) {
    if constexpr (debug_enabled) {
        log::internal::core_logger->debug(message);
    }
}

void info(const char *message) { log::internal::core_logger->info(message); }
void warn(const char *message) { log::internal::core_logger->warn(message); }
void error(const char *message) { log::internal::core_logger->error(message); }
void critical(const char *message) { log::internal::core_logger->critical(message); }

} // namespace core

namespace client {

void trace(const char *message) {
    if constexpr (debug_enabled) {
        log::internal::client_logger->trace(message);
    }
}

void debug(const char *message) {
    if constexpr (debug_enabled) {
        log::internal::client_logger->debug(message);
    }
}

void info(const char *message) { log::internal::client_logger->info(message); }
void warn(const char *message) { log::internal::client_logger->warn(message); }
void error(const char *message) { log::internal::client_logger->error(message); }
void critical(const char *message) { log::internal::client_logger->critical(message); }

} // namespace client

} // namespace muon
