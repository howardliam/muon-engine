#pragma once

#include <expected>
#include <filesystem>
#include <string_view>

namespace muon::utils {

struct DynamicLibraryError {
    const char *message;
};

auto openDynamicLibrary(const std::filesystem::path &path) -> std::expected<void *, DynamicLibraryError>;
auto loadSymbol(void *handle, const std::string_view name) -> std::expected<void *, DynamicLibraryError>;
auto closeDynamicLibrary(void *handle) -> std::expected<void, DynamicLibraryError>;

}
