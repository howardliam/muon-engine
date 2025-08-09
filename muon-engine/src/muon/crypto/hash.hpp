#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <fstream>
#include <string_view>

namespace muon::crypto {

enum class HashError {
    InitializationFailure,
    ProcessingFailure,
    FinalizationFailure,
};

constexpr uint64_t k_hashSize = 32;

auto hash(const uint8_t *data, size_t size) -> std::expected<std::array<uint8_t, k_hashSize>, HashError>;
auto hash(std::string_view string) -> std::expected<std::array<uint8_t, k_hashSize>, HashError>;

auto hash(std::ifstream &file) -> std::expected<std::array<uint8_t, k_hashSize>, HashError>;

} // namespace muon::crypto
