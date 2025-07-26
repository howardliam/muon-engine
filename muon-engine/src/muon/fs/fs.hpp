#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace muon::fs {

auto readFile(const std::filesystem::path &path) -> std::optional<std::string>;
auto readFileBinary(const std::filesystem::path &path) -> std::optional<std::vector<uint8_t>>;

auto writeFile(const uint8_t *data, size_t size, const std::filesystem::path &path) -> bool;

} // namespace muon::fs
