#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace muon::fs {

    [[nodiscard]] auto ReadFile(const std::filesystem::path &path) -> std::optional<std::vector<uint8_t>>;
    [[nodiscard]] auto WriteFile(const uint8_t *data, size_t size, const std::filesystem::path &path) -> bool;

}
