#pragma once

#include <format>
#include <print>
#include <string>

namespace muon::misc {

    class ILogger {
    public:
        template <typename... Args>
        void trace(std::format_string<Args ...> fmt, Args &&...args) {
            traceImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void debug(std::format_string<Args ...> fmt, Args &&...args) {
            debugImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void info(std::format_string<Args ...> fmt, Args &&...args) {
            infoImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void warn(std::format_string<Args ...> fmt, Args &&...args) {
            warnImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        template <typename... Args>
        void error(std::format_string<Args ...> fmt, Args &&...args) {
            errorImpl(std::format(fmt, std::forward<Args>(args)...));
        }

    private:
        virtual void traceImpl(std::string message) = 0;
        virtual void debugImpl(std::string message) = 0;
        virtual void infoImpl(std::string message) = 0;
        virtual void warnImpl(std::string message) = 0;
        virtual void errorImpl(std::string message) = 0;
    };

    class BasicLogger : public ILogger {
    public:
        BasicLogger();

    private:
        void traceImpl(std::string message) override;
        void debugImpl(std::string message) override;
        void infoImpl(std::string message) override;
        void warnImpl(std::string message) override;
        void errorImpl(std::string message) override;
    };
}
