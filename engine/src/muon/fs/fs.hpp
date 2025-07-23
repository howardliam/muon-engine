#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace muon::fs {

auto ReadFile(const std::filesystem::path &path) -> std::optional<std::string>;
auto ReadFileBinary(const std::filesystem::path &path) -> std::optional<std::vector<uint8_t>>;

auto WriteFile(const uint8_t *data, size_t size, const std::filesystem::path &path) -> bool;

} // namespace muon::fs
