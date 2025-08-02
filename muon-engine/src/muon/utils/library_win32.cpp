#include "muon/utils/library.hpp"

#include <expected>
#include <filesystem>
#include <windows.h>

namespace muon::utils {

auto openDynamicLibrary(const std::filesystem::path &path) -> std::expected<void *, DynamicLibraryError> {
    void *handle = LoadLibrary(path.c_str());
    if (!handle) {
        return std::unexpected(DynamicLibraryError{GetLastError()});
    }

    return handle;
}

auto loadSymbol(void *handle, const std::string_view name) -> std::expected<void *, DynamicLibraryError> {
    void *symbol = GetProcAddress(handle, name.data());
    if (!symbol) {
        return std::unexpected(DynamicLibraryError{GetLastError()});
    }

    return symbol;
}

auto closeDynamicLibrary(void *handle) -> std::expected<void, DynamicLibraryError> {
    int32_t result = FreeLibrary(handle);
    if (result != 0) {
        return std::unexpected(DynamicLibraryError{GetLastError()});
    }

    return {};
}

}
