#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

namespace muon::asset {

class Manager;

class Loader {
public:
    virtual ~Loader() = default;

    virtual auto FromMemory(const std::vector<uint8_t> &data) -> void = 0;
    virtual auto FromFile(const std::filesystem::path &path) -> void = 0;

    [[nodiscard]] virtual auto GetFileType() -> std::string_view = 0;
    auto SetManager(Manager *manager) -> void { m_manager = manager; }

protected:
    Manager *m_manager{nullptr};
};

} // namespace muon::asset
