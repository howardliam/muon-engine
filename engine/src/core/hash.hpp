#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <optional>
#include <string_view>

namespace {

constexpr size_t k_arraySize = 32;

}

namespace muon {

[[nodiscard]] auto HashString(const std::string_view string) -> std::optional<std::array<uint8_t, k_arraySize>>;
[[nodiscard]] auto HashFile(std::ifstream &file) -> std::optional<std::array<uint8_t, k_arraySize>>;

} // namespace muon
