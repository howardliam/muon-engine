#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <fstream>
#include <string_view>

namespace muon::crypto {

constexpr uint64_t hash_size = 32;

enum class hash_error {
    initialization_failure,
    processing_failure,
    finalization_failure,
};

auto hash(const uint8_t *data, size_t size) -> std::expected<std::array<uint8_t, hash_size>, hash_error>;
auto hash(std::string_view string) -> std::expected<std::array<uint8_t, hash_size>, hash_error>;

auto hash(std::ifstream &file) -> std::expected<std::array<uint8_t, hash_size>, hash_error>;

} // namespace muon::crypto
