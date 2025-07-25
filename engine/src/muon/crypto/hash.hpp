#pragma once

#include "tomcrypt.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <fstream>
#include <string>
#include <string_view>

namespace muon::crypto {

enum class HashError {
    StateCreationFailure,
    ProcessingFailure,
    FinalizationFailure,
};

template <size_t Size>
auto initStateForSize() -> std::expected<hash_state, HashError> {
    static_assert(Size == 32 || Size == 48 || Size == 64, "hash size must be 32, 48, or 64 bytes");

    hash_state state;

    int32_t result;
    if (Size == 32) {
        result = blake2b_256_init(&state);
    } else if (Size == 48) {
        result = blake2b_384_init(&state);
    } else if (Size == 64) {
        result = blake2b_512_init(&state);
    }

    if (result != CRYPT_OK) {
        return std::unexpected(HashError::StateCreationFailure);
    }
    return state;
}

template <size_t Size>
auto hashString(const std::string_view string) -> std::expected<std::array<uint8_t, Size>, HashError> {
    static_assert(Size == 32 || Size == 48 || Size == 64, "hash size must be 32, 48, or 64 bytes");

    auto stateResult = initStateForSize<Size>();
    if (!stateResult) {
        return std::unexpected(stateResult.error());
    }
    hash_state state = *stateResult;

    int32_t result = blake2b_process(&state, reinterpret_cast<const uint8_t *>(string.data()), string.size());
    if (result != CRYPT_OK) {
        return std::unexpected(HashError::ProcessingFailure);
    }

    std::array<uint8_t, Size> output;
    result = blake2b_done(&state, output.data());
    if (result != (CRYPT_OK)) {
        return std::unexpected(HashError::FinalizationFailure);
    }

    return output;
}

template <size_t Size>
auto hashFile(std::ifstream &file) -> std::expected<std::array<uint8_t, Size>, HashError> {
    static_assert(Size == 32 || Size == 48 || Size == 64, "hash size must be 32, 48, or 64 bytes");

    auto stateResult = initStateForSize<Size>();
    if (!stateResult) {
        return std::unexpected(stateResult.error());
    }
    hash_state state = *stateResult;

    // make sure to reset read position
    file.clear();
    file.seekg(0, std::ios::beg);

    std::string line;
    while (std::getline(file, line)) {
        int32_t result = blake2b_process(&state, reinterpret_cast<const uint8_t *>(line.data()), line.size());
        if (result != CRYPT_OK) {
            return std::unexpected(HashError::ProcessingFailure);
        }
    }

    std::array<uint8_t, Size> output;
    int32_t result = blake2b_done(&state, output.data());
    if (result != CRYPT_OK) {
        return std::unexpected(HashError::FinalizationFailure);
    }

    // make sure to reset read position
    file.clear();
    file.seekg(0, std::ios::beg);

    return output;
}

} // namespace muon::crypto
