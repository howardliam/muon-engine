#pragma once

#include "toml++/toml.hpp"

#include <atomic>
#include <filesystem>
#include <shared_mutex>

namespace muon::config {

class ConfigManager {
public:
    ConfigManager(const std::filesystem::path &configPath);
    ~ConfigManager();

    auto insert(const std::string_view key, const toml::table &table) -> void;
    auto write() -> void;

private:
    std::filesystem::path m_path;

    std::shared_mutex m_mutex;
    std::atomic<bool> m_dirty{false};

    toml::table m_config;
};

} // namespace muon::config
