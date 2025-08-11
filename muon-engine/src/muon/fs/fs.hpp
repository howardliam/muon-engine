#pragma once

#include "muon/core/buffer.hpp"
#include <expected>
#include <filesystem>
#include <string>

namespace muon::fs {

enum class RwError {
    FileNotFound,
    NotRegularFile,
    InsufficientPermissions,
    OpenFailure,
};

auto check_file(const std::filesystem::path &path) -> std::expected<void, RwError>;

auto read_file_text(const std::filesystem::path &path) -> std::expected<std::string, RwError>;
auto read_file_binary(const std::filesystem::path &path) -> std::expected<Buffer, RwError>;

auto write_file_text(std::string_view text, const std::filesystem::path &path) -> std::expected<void, RwError>;
auto write_file_binary(BufferView buffer, const std::filesystem::path &path) -> std::expected<void, RwError>;

} // namespace muon::fs
