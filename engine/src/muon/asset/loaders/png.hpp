#pragma once

#include "muon/asset/loader.hpp"

namespace muon::asset {

class PngLoader final : public Loader {
public:
    [[nodiscard]] virtual auto GetFileTypes() const -> std::set<std::string_view> override;

    virtual auto FromMemory(const std::vector<uint8_t> &data) -> void override;
    virtual auto FromFile(const std::filesystem::path &path) -> void override;
};

} // namespace muon::asset
