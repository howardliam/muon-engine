#pragma once

#include <filesystem>
#include <string_view>
#include <vector>

namespace muon::asset {

class Manager;

class Loader {
public:
    Loader(Manager *manager) : m_manager{manager} {}
    virtual ~Loader() = default;

    [[nodiscard]] virtual auto GetFileType() -> std::string_view = 0;

    virtual auto FromMemory(const std::vector<uint8_t> &data) -> void = 0;
    virtual auto FromFile(const std::filesystem::path &path) -> void = 0;

protected:
    Manager *m_manager{nullptr};
};

} // namespace muon::asset
