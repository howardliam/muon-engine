#pragma once

#include <filesystem>
#include <set>
#include <string_view>
#include <vector>

namespace muon::asset {

class AssetManager;

class AssetLoader {
public:
    virtual ~AssetLoader() = default;

    virtual auto getFileTypes() const -> std::set<std::string_view> = 0;
    auto SetAssetManager(AssetManager *manager) -> void { m_manager = manager; }

    auto operator==(const AssetLoader *other) -> bool { return getFileTypes() == other->getFileTypes(); }

    virtual auto fromMemory(const std::vector<uint8_t> &data) -> void = 0;
    virtual auto fromFile(const std::filesystem::path &path) -> void = 0;

protected:
    AssetManager *m_manager{nullptr};
};

} // namespace muon::asset
