#include "muon/config/config_manager.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"

#include <atomic>
#include <filesystem>
#include <fstream>

namespace muon::config {

ConfigManager::ConfigManager(const std::filesystem::path &path) : m_path{path} {
    auto validatePath = [](const std::filesystem::path &path) -> bool {
        if (!std::filesystem::exists(path)) {
            return false;
        }

        auto perms = std::filesystem::status(path).permissions();

        auto canRead = (perms & std::filesystem::perms::owner_read) != std::filesystem::perms::none;
        auto canWrite = (perms & std::filesystem::perms::owner_write) != std::filesystem::perms::none;

        return canRead && canWrite;
    };

    bool valid = false;
    if (std::filesystem::exists(m_path)) {
        valid = validatePath(m_path);
    } else if (std::filesystem::exists(m_path.parent_path())) {
        valid = validatePath(m_path.parent_path());
    }
    core::expect(valid, "the program must be able to read/write file at: {}", m_path.c_str());

    core::debug("created config manager, with config file at: {}", m_path.c_str());
}

ConfigManager::~ConfigManager() {
    auto writeResult = write();
    if (!writeResult) {
        core::debug("config not written before destruction");
    }

    core::debug("destroyed config manager");
}

auto ConfigManager::insert(const std::string_view key, const toml::table &table) -> void {
    std::unique_lock<std::shared_mutex> lock{m_mutex};

    auto [_, inserted] = m_config.insert_or_assign(key, table);
    m_dirty.store(true, std::memory_order::release);

    core::trace("{} data at key: {}", inserted ? "inserted" : "assigned", key);
}

auto ConfigManager::write() -> std::expected<void, ConfigManagerError> {
    if (!m_dirty.load(std::memory_order::acquire)) {
        core::trace("skipped writing config; unchanged since last write");
        return std::unexpected(ConfigManagerError::ConfigNotModified);
    }

    std::shared_lock<std::shared_mutex> lock{m_mutex};

    std::ofstream configFile{m_path, std::ios::trunc};
    if (!configFile.is_open()) {
        core::trace("skipped writing config; failed to open file");
        return std::unexpected(ConfigManagerError::FailedToOpenFile);
    }

    const auto &config = m_config;

    configFile << config << std::endl;
    m_dirty.store(false, std::memory_order::release);

    core::trace("wrote out config file to disk");

    return {};
}

} // namespace muon::config
