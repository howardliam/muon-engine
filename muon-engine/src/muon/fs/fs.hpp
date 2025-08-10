#pragma once

#include "muon/core/buffer.hpp"
#include <expected>
#include <filesystem>
#include <string>

namespace muon::fs {

enum class rw_error {
    file_not_found,
    not_regular_file,
    insufficient_permissions,
    open_failure,
};

auto check_file(const std::filesystem::path &path) -> std::expected<void, rw_error>;

auto read_file_text(const std::filesystem::path &path) -> std::expected<std::string, rw_error>;
auto read_file_binary(const std::filesystem::path &path) -> std::expected<buffer, rw_error>;

auto write_file_text(std::string_view text, const std::filesystem::path &path) -> std::expected<void, rw_error>;
auto write_file_binary(const buffer &buffer, const std::filesystem::path &path) -> std::expected<void, rw_error>;

} // namespace muon::fs
