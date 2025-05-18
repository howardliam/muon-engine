#pragma once

#include <muon/common/log/logger.hpp>

class Logger final : public muon::common::log::ILogger {
public:
    Logger();

private:
    void traceImpl(const std::string_view message) override;
    void debugImpl(const std::string_view message) override;
    void infoImpl(const std::string_view message) override;
    void warnImpl(const std::string_view message) override;
    void errorImpl(const std::string_view message) override;
};
