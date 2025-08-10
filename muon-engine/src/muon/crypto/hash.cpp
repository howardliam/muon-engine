#include "muon/crypto/hash.hpp"

#include "fmt/format.h"
#include "fmt/ranges.h"
#include "muon/core/buffer.hpp"
#include "sodium/crypto_generichash.h"

namespace muon::crypto {

hash::hash() : raw_buffer{32} {}
hash::~hash() { release(); }

auto hash::to_string() const -> std::string {
    return fmt::format("{:02x}", fmt::join(begin(), end(), ""));
}

auto hash::from_buffer(const raw_buffer &buffer) -> std::expected<hash, hash_error> {
    hash output;
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

auto hash::from_text(std::string_view text) -> std::expected<hash, hash_error> {
    hash output;
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

auto hash::from_file(std::ifstream &file) -> std::expected<hash, hash_error> {
    crypto_generichash_state state;

    int32_t result = crypto_generichash_init(&state, nullptr, 0, 32);
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

    hash output;
    result = crypto_generichash_final(&state, output.data(), output.size());
    if (result != 0) {
        return std::unexpected(hash_error::finalization_failure);
    }

    return output;
}

} // namespace muon::crypto

auto fmt::formatter<muon::crypto::hash>::format(const muon::crypto::hash &hash, format_context &ctx) const -> format_context::iterator {
    return formatter<string_view>::format(hash.to_string(), ctx);
}
