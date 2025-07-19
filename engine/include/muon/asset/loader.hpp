#pragma once

#include <filesystem>
#include <set>
#include <string_view>
#include <vector>

namespace muon::asset {

class Manager;

class Loader {
public:
    virtual ~Loader() = default;

    virtual auto GetFileTypes() const -> std::set<std::string_view> = 0;
    auto SetManager(Manager *manager) -> void { m_manager = manager; }

    auto operator==(const Loader *other) -> bool { return GetFileTypes() == other->GetFileTypes(); }

    virtual auto FromMemory(const std::vector<uint8_t> &data) -> void = 0;
    virtual auto FromFile(const std::filesystem::path &path) -> void = 0;

protected:
    Manager *m_manager{nullptr};
};

} // namespace muon::asset
