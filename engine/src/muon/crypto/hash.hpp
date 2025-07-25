#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
namespace muon::crypto {

constexpr size_t k_hashSize = 32;

auto hashString(const std::string_view string) -> std::optional<std::array<uint8_t, k_hashSize>>;
auto hashFile(std::ifstream &file) -> std::optional<std::array<uint8_t, k_hashSize>>;

} // namespace muon::crypto
