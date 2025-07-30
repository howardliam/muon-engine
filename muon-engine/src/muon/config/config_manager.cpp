#include "muon/config/config_manager.hpp"

#include "muon/core/log.hpp"

#include <fstream>

namespace muon::config {

ConfigManager::ConfigManager(const std::filesystem::path &configPath) : m_path{configPath} {
    core::debug("created config manager");
}

ConfigManager::~ConfigManager() {
    if (m_dirty) {
        write();
    }
    core::debug("destroyed config manager");
}

auto ConfigManager::insert(const std::string_view key, const toml::table &table) -> void {
    m_mutex.lock();

    auto [_, inserted] = m_config.insert_or_assign(key, table);
    m_dirty = true;

    core::trace("{} data at key: {}", inserted ? "inserted" : "assigned", key);
}

auto ConfigManager::write() -> void {
    m_mutex.lock();

    std::ofstream configFile{m_path, std::ios::trunc};
    if (!configFile.is_open()) {
        return;
    }

    configFile << m_config << std::endl;
    m_dirty = false;

    core::trace("wrote out config file to disk");
}

} // namespace muon::config
