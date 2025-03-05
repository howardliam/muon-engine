#include "logging.hpp"

#include <algorithm>
#include <cctype>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <memory>
#include <spdlog/common.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>
#include <toml++/impl/array.hpp>
#include <vector>

#include "core/compression.hpp"

namespace muon::logging {

    namespace defaults {
        constexpr std::string directory = "muon/logs/";
        constexpr std::string fileLevel = "trace";
        constexpr std::string stdoutLevel = "warn";

        constexpr std::string_view fileFormat = "[%Y-%m-%d %T.%e%z] [%8l] [%8n]: %v";
        constexpr std::string_view stdoutFormat = "[%Y-%m-%d %T.%e%z] [%^%8l%$] [%8n]: %v";
    }

    void compressLatestLog(std::string_view logFilename) {
        std::filesystem::path latestLogPath = std::format("{}{}.log", defaults::directory, logFilename);

        std::ifstream latestLogStream{latestLogPath};
        if (!latestLogStream.good()) {
            return;
        }

        auto lastWriteTime = [&latestLogPath]() -> std::string {
            auto time = std::filesystem::last_write_time(latestLogPath);
            return std::format("{:%FT%H:%M:%S}", time);
        }();
        std::filesystem::path compressedLogPath = std::format("{}{}.{}.log.zst", defaults::directory, lastWriteTime, logFilename);

        std::ofstream compressedLogStream{compressedLogPath, std::ios::binary};

        try {
            compression::compressFile(latestLogStream, compressedLogStream);
        } catch (std::exception &e) {
            spdlog::error(e.what());
        }

        latestLogStream.open(latestLogPath, std::ios::trunc | std::ios::out);
    }

    void deleteOldLogs(size_t maxHistory) {
        std::vector<std::filesystem::directory_entry> files;
        for (const auto &file : std::filesystem::directory_iterator(defaults::directory)) {
            if (std::filesystem::is_regular_file(file)) {
                files.push_back(file);
            }
        }

        auto sortByLastWrite = [](std::filesystem::directory_entry &a, std::filesystem::directory_entry &b) {
            return std::filesystem::last_write_time(a) < std::filesystem::last_write_time(b);
        };
        std::sort(files.begin(), files.end(), sortByLastWrite);

        while (files.size() > maxHistory) {
            std::filesystem::remove(files.front());
            files.erase(files.begin());
        }
    }

    void parseLogConfig(toml::table &config, LogInfo &logInfo) {
        auto directory = config["global"]["directory"].value<std::string>().value_or(defaults::directory);
        if (!directory.ends_with('/')) {
            directory.push_back('/');
        }

        logInfo.loggers.clear();

        auto loggers = config["loggers"].as_array();

        for (const auto &logger : *loggers) {
            auto &loggerInfoTable = *logger.as_table();

            LoggerInfo loggerInfo{};

            if (!loggerInfoTable.contains("name")) {
                spdlog::warn("Logger without `name` in config, skipping");
                continue;
            }

            loggerInfo.name = loggerInfoTable["name"].value<std::string>().value();

            if (loggerInfoTable.contains("file")) {
                auto &fileSinkTable = *loggerInfoTable["file"].as_table();

                FileSinkInfo sinkInfo{};

                auto filename = fileSinkTable["filename"].value<std::string>();
                if (!filename.has_value()) {
                    spdlog::warn("{} logger's file sink lacks a filename, skipping", loggerInfo.name);
                    continue;
                }
                sinkInfo.filename = filename.value();

                auto level = fileSinkTable["level"].value<std::string>();
                if (!level.has_value()) {
                    spdlog::warn("{} logger's file sink log level is not set, using default", loggerInfo.name);
                }
                sinkInfo.level = spdlog::level::from_str(level.value_or(defaults::fileLevel));

                auto format = fileSinkTable["format"].value<std::string>();
                if (!format.has_value()) {
                    spdlog::warn("{} logger's file sink format is not set, using default", loggerInfo.name);
                }
                sinkInfo.format = format.value_or(defaults::fileFormat.data());

                sinkInfo.directory = directory;

                loggerInfo.fileSink = sinkInfo;
            }

            if (loggerInfoTable.contains("stdout")) {
                auto &stdoutSinkTable = *loggerInfoTable["stdout"].as_table();

                StdoutSinkInfo sinkInfo{};

                auto level = stdoutSinkTable["level"].value<std::string>();
                if (!level.has_value()) {
                    spdlog::warn("{} logger's stdout sink log level is not set, using default", loggerInfo.name);
                }
                sinkInfo.level = spdlog::level::from_str(level.value_or(defaults::stdoutLevel));

                auto format = stdoutSinkTable["format"].value<std::string>();
                if (!format.has_value()) {
                    spdlog::warn("{} logger's stdout sink format is not set, using default", loggerInfo.name);
                }
                sinkInfo.format = format.value_or(defaults::stdoutFormat.data());

                loggerInfo.stdoutSink = sinkInfo;
            }

            logInfo.loggers.insert(std::make_pair(loggerInfo.name, loggerInfo));
        }
    }

    Logger createLogger(LoggerInfo &loggerInfo) {
        std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks;

        if (loggerInfo.fileSink.has_value()) {
            auto fileSinkInfo = loggerInfo.fileSink.value();

            std::filesystem::path logPath = std::format("{}{}.log", fileSinkInfo.directory, fileSinkInfo.filename);

            auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath, true);
            fileSink->set_level(fileSinkInfo.level);
            fileSink->set_pattern(fileSinkInfo.format);

            sinks.push_back(fileSink);
        }

        if (loggerInfo.stdoutSink.has_value()) {
            auto stdoutSinkInfo = loggerInfo.stdoutSink.value();

            auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            stdoutSink->set_level(stdoutSinkInfo.level);
            stdoutSink->set_pattern(stdoutSinkInfo.format);

            sinks.push_back(stdoutSink);
        }

        auto logger = std::make_shared<spdlog::logger>(loggerInfo.name, sinks.begin(), sinks.end());
        logger->set_level(spdlog::level::trace);

        return logger;
    }
}
