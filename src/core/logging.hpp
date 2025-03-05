#pragma once

#include <map>
#include <memory>
#include <optional>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>
#include <toml++/toml.hpp>

namespace muon::logging {

    using Logger = std::shared_ptr<spdlog::logger>;

    struct FileSinkInfo {
        spdlog::level::level_enum level;
        std::string format;
        std::string filename;
        std::string directory;
    };

    struct StdoutSinkInfo {
        spdlog::level::level_enum level;
        std::string format;
    };

    struct LoggerInfo {
        std::string name;
        std::optional<FileSinkInfo> fileSink;
        std::optional<StdoutSinkInfo> stdoutSink;
    };

    struct LogInfo {
        std::map<std::string, LoggerInfo> loggers;
    };

    void parseLogConfig(toml::table &config, LogInfo &globalInfo);
    Logger createLogger(LoggerInfo &loggerInfo);
}
