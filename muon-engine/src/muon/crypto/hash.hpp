#pragma once

#include "fmt/base.h"
#include "muon/core/buffer.hpp"

#include <expected>
#include <fstream>
#include <string_view>

namespace muon::crypto {

enum class hash_error {
    initialization_failure,
    processing_failure,
    finalization_failure,
};

struct hash : public raw_buffer {
    hash();
    ~hash();

    auto to_string() const -> std::string;

    static auto from_buffer(const raw_buffer &buffer) -> std::expected<hash, hash_error>;
    static auto from_text(std::string_view text) -> std::expected<hash, hash_error>;
    static auto from_file(std::ifstream &file) -> std::expected<hash, hash_error>;
};

} // namespace muon::crypto

template <>
struct fmt::formatter<muon::crypto::hash> : formatter<string_view> {
    auto format(const muon::crypto::hash &hash, format_context &ctx) const -> format_context::iterator;
};
