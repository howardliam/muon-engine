#include "muon/config/config_manager.hpp"

#include "muon/core/log.hpp"

#include <atomic>
#include <fstream>

namespace muon::config {

ConfigManager::ConfigManager(const std::filesystem::path &configPath) : m_path{configPath} {
    core::debug("created config manager");
}

ConfigManager::~ConfigManager() {
    write();
    core::debug("destroyed config manager");
}

auto ConfigManager::insert(const std::string_view key, const toml::table &table) -> void {
    std::unique_lock<std::shared_mutex> lock{m_mutex};

    auto [_, inserted] = m_config.insert_or_assign(key, table);
    m_dirty.store(true, std::memory_order::release);

    core::trace("{} data at key: {}", inserted ? "inserted" : "assigned", key);
}

auto ConfigManager::write() -> void {
    if (!m_dirty.load(std::memory_order::acquire)) {
        core::trace("skipped writing config; unchanged since last write");
        return;
    }

    std::shared_lock<std::shared_mutex> lock{m_mutex};

    if (!m_dirty.load(std::memory_order::relaxed)) {
        core::trace("skipped writing config; config updated by another thread");
        return;
    }

    std::ofstream configFile{m_path, std::ios::trunc};
    if (!configFile.is_open()) {
        return;
    }

    const auto &config = m_config;

    configFile << config << std::endl;
    m_dirty.store(false, std::memory_order::release);

    core::trace("wrote out config file to disk");
}

} // namespace muon::config
