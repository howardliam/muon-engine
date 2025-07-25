#pragma once

#include "muon/asset/loader.hpp"

namespace muon::asset {

class PngLoader final : public Loader {
public:
    [[nodiscard]] virtual auto getFileTypes() const -> std::set<std::string_view> override;

    virtual auto fromMemory(const std::vector<uint8_t> &data) -> void override;
    virtual auto fromFile(const std::filesystem::path &path) -> void override;
};

} // namespace muon::asset
