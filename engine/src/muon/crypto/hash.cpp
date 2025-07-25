#include "muon/crypto/hash.hpp"

#include "tomcrypt.h"

#include <fstream>
#include <string>

namespace muon::crypto {

auto hashString(const std::string_view string) -> std::optional<std::array<uint8_t, k_hashSize>> {
    hash_state state;
    blake2b_256_init(&state);

    int32_t result = blake2b_process(&state, reinterpret_cast<const uint8_t *>(string.data()), string.size());
    if (result != CRYPT_OK) {
        return std::nullopt;
    }

    std::array<uint8_t, k_hashSize> output;
    result = blake2b_done(&state, output.data());
    if (result != (CRYPT_OK)) {
        return std::nullopt;
    }

    return output;
}

auto hashFile(std::ifstream &file) -> std::optional<std::array<uint8_t, k_hashSize>> {
    hash_state state;
    blake2b_256_init(&state);

    int32_t result;

    std::string line;
    while (std::getline(file, line)) {
        result = blake2b_process(&state, reinterpret_cast<const uint8_t *>(line.data()), line.size());
        if (result != CRYPT_OK) {
            return std::nullopt;
        }
    }

    std::array<uint8_t, k_hashSize> output;
    result = blake2b_done(&state, output.data());
    if (result != (CRYPT_OK)) {
        return std::nullopt;
    }

    return output;
}

} // namespace muon::crypto
