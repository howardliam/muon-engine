#include "muon/core/hash.hpp"

#include <tomcrypt.h>

namespace muon {

auto HashString(const std::string_view string) -> std::optional<std::array<uint8_t, k_arraySize>> {
    hash_state state;
    blake2b_256_init(&state);

    int32_t result = blake2b_process(&state, reinterpret_cast<const uint8_t *>(string.data()), string.size());
    if (result != CRYPT_OK) {
        return std::nullopt;
    }

    std::array<uint8_t, k_arraySize> output;
    result = blake2b_done(&state, output.data());
    if (result != (CRYPT_OK)) {
        return std::nullopt;
    }

    return output;
}

auto HashFile(std::ifstream &file) -> std::optional<std::array<uint8_t, k_arraySize>> {
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

    std::array<uint8_t, k_arraySize> output;
    result = blake2b_done(&state, output.data());
    if (result != (CRYPT_OK)) {
        return std::nullopt;
    }

    return output;
}

} // namespace muon
