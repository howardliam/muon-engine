#pragma once

#include <format>
#include <string_view>

namespace mu::common::log {

    class ILogger {
    public:
        template <typename... Args>
        void trace(std::format_string<Args ...> fmt, Args &&...args) {
            traceImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        void trace(const std::string_view message) {
            traceImpl(message);
        }

        template <typename... Args>
        void debug(std::format_string<Args ...> fmt, Args &&...args) {
            debugImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        void debug(const std::string_view message) {
            debugImpl(message);
        }

        template <typename... Args>
        void info(std::format_string<Args ...> fmt, Args &&...args) {
            infoImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        void info(const std::string_view message) {
            infoImpl(message);
        }

        template <typename... Args>
        void warn(std::format_string<Args ...> fmt, Args &&...args) {
            warnImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        void warn(const std::string_view message) {
            warnImpl(message);
        }

        template <typename... Args>
        void error(std::format_string<Args ...> fmt, Args &&...args) {
            errorImpl(std::format(fmt, std::forward<Args>(args)...));
        }

        void error(const std::string_view message) {
            errorImpl(message);
        }

    private:
        virtual void traceImpl(const std::string_view message) = 0;
        virtual void debugImpl(const std::string_view message) = 0;
        virtual void infoImpl(const std::string_view message) = 0;
        virtual void warnImpl(const std::string_view message) = 0;
        virtual void errorImpl(const std::string_view message) = 0;
    };

    class BasicLogger final : public ILogger {
    private:
        void traceImpl(const std::string_view message) override;
        void debugImpl(const std::string_view message) override;
        void infoImpl(const std::string_view message) override;
        void warnImpl(const std::string_view message) override;
        void errorImpl(const std::string_view message) override;
    };

}
