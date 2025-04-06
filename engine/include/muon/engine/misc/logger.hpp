#pragma once

#include <format>
#include <iostream>
#include <print>
#include <string>
#include <utility>

//std::format_string<Args ...> fmt, Args &&...args

namespace muon::engine::misc {
    class ILogger {
    public:
        virtual void traceImpl(std::string message) = 0;

        template <typename... Args>
        void trace(std::format_string<Args ...> fmt, Args &&...args) {
            traceImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        virtual void debugImpl(std::string message) = 0;

        template <typename... Args>
        void debug(std::format_string<Args ...> fmt, Args &&...args) {
            debugImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        virtual void infoImpl(std::string message) = 0;

        template <typename... Args>
        void info(std::format_string<Args ...> fmt, Args &&...args) {
            infoImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        virtual void warnImpl(std::string message) = 0;

        template <typename... Args>
        void warn(std::format_string<Args ...> fmt, Args &&...args) {
            warnImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        virtual void errorImpl(std::string message) = 0;

        template <typename... Args>
        void error(std::format_string<Args ...> fmt, Args &&...args) {
            errorImpl(std::format(fmt, std::forward<Args>(args)...));
        }
    };
}
