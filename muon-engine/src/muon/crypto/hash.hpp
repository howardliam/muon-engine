#pragma once

#include "fmt/base.h"
#include "muon/core/buffer.hpp"

#include <expected>
#include <fstream>
#include <string_view>

namespace muon::crypto {

enum class HashError {
    InitializationFailuer,
    ProcessingFailure,
    FinalizationFailure,
};

struct Hash : public Buffer {
    Hash();

    auto to_string() const -> std::string;

    static auto from_buffer(BufferView buffer) -> std::expected<Hash, HashError>;
    static auto from_text(std::string_view text) -> std::expected<Hash, HashError>;
    static auto from_file(std::ifstream &file) -> std::expected<Hash, HashError>;
};

} // namespace muon::crypto

template <>
struct fmt::formatter<muon::crypto::Hash> : formatter<string_view> {
    auto format(const muon::crypto::Hash &hash, format_context &ctx) const -> format_context::iterator;
};
