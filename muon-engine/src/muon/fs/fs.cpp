#include "muon/fs/fs.hpp"

#include <expected>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace muon::fs {

auto check_file(const std::filesystem::path &path) -> std::expected<void, rw_error> {
    if (!std::filesystem::exists(path)) {
        return std::unexpected(rw_error::file_not_found);
    }

    if (!std::filesystem::is_regular_file(path)) {
        return std::unexpected(rw_error::not_regular_file);
    }

    auto permissions = std::filesystem::status(path).permissions();
    bool can_read = (permissions & std::filesystem::perms::owner_read) != std::filesystem::perms::none;
    bool can_write = (permissions & std::filesystem::perms::owner_write) != std::filesystem::perms::none;

    if (!can_read && !can_write) {
        return std::unexpected(rw_error::insufficient_permissions);
    }

    return {};
}

auto read_file_text(const std::filesystem::path &path) -> std::expected<std::string, rw_error> {
    auto result = check_file(path);
    if (!result) {
        return std::unexpected(result.error());
    }

    std::ifstream file{path};
    if (!file) {
        return std::unexpected(rw_error::open_failure);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

auto read_file_binary(const std::filesystem::path &path) -> std::expected<buffer, rw_error> {
    auto result = check_file(path);
    if (!result) {
        return std::unexpected(result.error());
    }

    std::ifstream file{path, std::ios::ate | std::ios::binary};
    if (!file) {
        return std::unexpected(rw_error::open_failure);
    }

    buffer buffer(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

    return buffer;
}

auto write_file_text(std::string_view text, const std::filesystem::path &path) -> std::expected<void, rw_error> {
    auto result = check_file(path);
    if (!result) {
        return std::unexpected(result.error());
    }

    std::ofstream file{path};
    if (!file.is_open()) {
        return std::unexpected(rw_error::open_failure);
    }

    file.write(text.data(), text.size());
    return {};
}

auto write_file_binary(const buffer &buffer, const std::filesystem::path &path) -> std::expected<void, rw_error> {
    auto result = check_file(path);
    if (!result) {
        return std::unexpected(result.error());
    }

    std::ofstream file{path, std::ios::binary};
    if (!file.is_open()) {
        return std::unexpected(rw_error::open_failure);
    }

    file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
    return {};
}

} // namespace muon::fs
