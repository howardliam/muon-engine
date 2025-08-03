#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <fstream>

namespace muon::crypto {

enum class HashError {
    InitializationFailure,
    ProcessingFailure,
    FinalizationFailure,
};

auto hash(const uint8_t *data, size_t size) -> std::expected<std::array<uint8_t, 32>, HashError>;
auto hash(std::ifstream &file) -> std::expected<std::array<uint8_t, 32>, HashError>;

} // namespace muon::crypto
