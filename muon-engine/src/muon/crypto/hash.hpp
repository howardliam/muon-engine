#pragma once

#include "muon/core/buffer.hpp"
#include <array>
#include <cstdint>
#include <expected>
#include <fstream>
#include <string_view>

namespace muon::crypto {

constexpr size_t hash_size = 32;
using hash_result = std::array<uint8_t, hash_size>;

enum class hash_error {
    initialization_failure,
    processing_failure,
    finalization_failure,
};

auto hash(const buffer &buffer) -> std::expected<hash_result, hash_error>;
auto hash(std::string_view text) -> std::expected<hash_result, hash_error>;

auto hash(std::ifstream &file) -> std::expected<hash_result, hash_error>;

} // namespace muon::crypto
