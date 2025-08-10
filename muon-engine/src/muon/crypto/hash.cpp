#include "muon/crypto/hash.hpp"

#include "sodium/crypto_generichash.h"

namespace muon::crypto {

auto hash(const raw_buffer &buffer) -> std::expected<hash_result, hash_error> {
    hash_result output;
    int32_t result = crypto_generichash(
        output.data(), output.size(),
        buffer.data(), buffer.size(),
        nullptr, 0
    );

    if (result != 0) {
        return std::unexpected(hash_error::processing_failure);
    }

    return output;
}

auto hash(std::string_view text) -> std::expected<hash_result, hash_error> {
    hash_result output;
    int32_t result = crypto_generichash(
        output.data(), output.size(),
        reinterpret_cast<const uint8_t *>(text.data()), text.size(),
        nullptr, 0
    );

    if (result != 0) {
        return std::unexpected(hash_error::processing_failure);
    }

    return output;
}

auto hash(std::ifstream &file) -> std::expected<hash_result, hash_error> {
    crypto_generichash_state state;

    int32_t result = crypto_generichash_init(&state, nullptr, 0, hash_size);
    if (result != 0) {
        return std::unexpected(hash_error::initialization_failure);
    }

    // make sure to reset read position
    file.clear();
    file.seekg(0, std::ios::beg);

    std::string line;
    while (std::getline(file, line)) {
        result = crypto_generichash_update(&state, reinterpret_cast<const uint8_t *>(line.data()), line.size());
        if (result != 0) {
            return std::unexpected(hash_error::processing_failure);
        }
    }

    // make sure to reset read position
    file.clear();
    file.seekg(0, std::ios::beg);

    hash_result output;
    result = crypto_generichash_final(&state, output.data(), output.size());
    if (result != 0) {
        return std::unexpected(hash_error::finalization_failure);
    }

    return output;
}

} // namespace muon::crypto
