#pragma once

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

    auto to_string() -> std::string;

    static auto from_buffer(const raw_buffer &buffer) -> std::expected<hash, hash_error>;
    static auto from_text(std::string_view text) -> std::expected<hash, hash_error>;
    static auto from_file(std::ifstream &file) -> std::expected<hash, hash_error>;
};

} // namespace muon::crypto
