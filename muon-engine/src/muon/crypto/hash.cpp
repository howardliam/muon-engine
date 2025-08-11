#include "muon/crypto/hash.hpp"

#include "fmt/format.h"
#include "fmt/ranges.h"
#include "muon/core/buffer.hpp"
#include "sodium/crypto_generichash.h"

namespace muon::crypto {

Hash::Hash() : Buffer{32} {}

auto Hash::to_string() const -> std::string {
    return fmt::format("{:02x}", fmt::join(begin(), end(), ""));
}

auto Hash::from_buffer(BufferView buffer) -> std::expected<Hash, HashError> {
    Hash output;
    int32_t result = crypto_generichash(
        output.data(), output.size(),
        buffer.data(), buffer.size(),
        nullptr, 0
    );

    if (result != 0) {
        return std::unexpected(HashError::ProcessingFailure);
    }

    return output;
}

auto Hash::from_text(std::string_view text) -> std::expected<Hash, HashError> {
    Hash output;
    int32_t result = crypto_generichash(
        output.data(), output.size(),
        reinterpret_cast<const uint8_t *>(text.data()), text.size(),
        nullptr, 0
    );

    if (result != 0) {
        return std::unexpected(HashError::ProcessingFailure);
    }

    return output;
}

auto Hash::from_file(std::ifstream &file) -> std::expected<Hash, HashError> {
    crypto_generichash_state state;

    int32_t result = crypto_generichash_init(&state, nullptr, 0, 32);
    if (result != 0) {
        return std::unexpected(HashError::InitializationFailuer);
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

    Hash output;
    result = crypto_generichash_final(&state, output.data(), output.size());
    if (result != 0) {
        return std::unexpected(HashError::FinalizationFailure);
    }

    return output;
}

} // namespace muon::crypto

auto fmt::formatter<muon::crypto::Hash>::format(const muon::crypto::Hash &hash, format_context &ctx) const -> format_context::iterator {
    return formatter<string_view>::format(hash.to_string(), ctx);
}
