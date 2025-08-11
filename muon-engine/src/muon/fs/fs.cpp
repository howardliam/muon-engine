#include "muon/fs/fs.hpp"

#include <expected>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace muon::fs {

auto check_file(const std::filesystem::path &path) -> std::expected<void, RwError> {
    if (!std::filesystem::exists(path)) {
        return std::unexpected(RwError::FileNotFound);
    }

    if (!std::filesystem::is_regular_file(path)) {
        return std::unexpected(RwError::NotRegularFile);
    }

    auto permissions = std::filesystem::status(path).permissions();
    bool can_read = (permissions & std::filesystem::perms::owner_read) != std::filesystem::perms::none;
    bool can_write = (permissions & std::filesystem::perms::owner_write) != std::filesystem::perms::none;

    if (!can_read && !can_write) {
        return std::unexpected(RwError::InsufficientPermissions);
    }

    return {};
}

auto read_file_text(const std::filesystem::path &path) -> std::expected<std::string, RwError> {
    auto result = check_file(path);
    if (!result) {
        return std::unexpected(result.error());
    }

    std::ifstream file{path};
    if (!file) {
        return std::unexpected(RwError::OpenFailure);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

auto read_file_binary(const std::filesystem::path &path) -> std::expected<Buffer, RwError> {
    auto result = check_file(path);
    if (!result) {
        return std::unexpected(result.error());
    }

    std::ifstream file{path, std::ios::ate | std::ios::binary};
    if (!file) {
        return std::unexpected(RwError::OpenFailure);
    }

    Buffer buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

    return buffer;
}

auto write_file_text(std::string_view text, const std::filesystem::path &path) -> std::expected<void, RwError> {
    auto result = check_file(path);
    if (!result) {
        return std::unexpected(result.error());
    }

    std::ofstream file{path};
    if (!file.is_open()) {
        return std::unexpected(RwError::OpenFailure);
    }

    file.write(text.data(), text.size());
    return {};
}

auto write_file_binary(BufferView buffer, const std::filesystem::path &path) -> std::expected<void, RwError> {
    auto result = check_file(path);
    if (!result) {
        return std::unexpected(result.error());
    }

    std::ofstream file{path, std::ios::binary};
    if (!file.is_open()) {
        return std::unexpected(RwError::OpenFailure);
    }

    file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
    return {};
}

} // namespace muon::fs
