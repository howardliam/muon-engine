#include "muon/crypto/hash.hpp"

#include "sodium/crypto_generichash.h"

namespace muon::crypto {

auto hash(const uint8_t *data, size_t size) -> std::expected<std::array<uint8_t, 32>, HashError> {
    std::array<uint8_t, 32> output;
    int32_t result = crypto_generichash(output.data(), output.size(), data, size, nullptr, 0);
    if (result != 0) {
        return std::unexpected(HashError::ProcessingFailure);
    }

    return output;
}

auto hash(std::string_view string) -> std::expected<std::array<uint8_t, k_hashSize>, HashError> {
    return hash(reinterpret_cast<const uint8_t *>(string.data()), string.size());
}

auto hash(std::ifstream &file) -> std::expected<std::array<uint8_t, 32>, HashError> {
    crypto_generichash_state state;

    int32_t result = crypto_generichash_init(&state, nullptr, 0, 32);
    if (result != 0) {
        return std::unexpected(HashError::InitializationFailure);
    }

    // make sure to reset read position
    file.clear();
    file.seekg(0, std::ios::beg);

    std::string line;
    while (std::getline(file, line)) {
        result = crypto_generichash_update(&state, reinterpret_cast<const uint8_t *>(line.data()), line.size());
        if (result != 0) {
            return std::unexpected(HashError::ProcessingFailure);
        }
    }

    // make sure to reset read position
    file.clear();
    file.seekg(0, std::ios::beg);

    std::array<uint8_t, 32> output;
    result = crypto_generichash_final(&state, output.data(), output.size());
    if (result != 0) {
        return std::unexpected(HashError::FinalizationFailure);
    }

    return output;
}

} // namespace muon::crypto
