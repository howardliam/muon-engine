#include "muon/utils/platform.hpp"

#include "muon/core/log.hpp"

#include <csignal>
#include <dlfcn.h>
#include <unistd.h>

namespace muon::utils {

auto open_library(const std::filesystem::path &path) -> std::expected<LibraryHandle, LibraryError> {
    LibraryHandle handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        core::error(dlerror());
        return std::unexpected(LibraryError::LibraryOpenFailure);
    }

    return handle;
}

auto load_symbol(LibraryHandle handle, std::string_view name) -> std::expected<SymbolHandle, LibraryError> {
    SymbolHandle symbol = dlsym(handle, name.data());
    if (!symbol) {
        core::error(dlerror());
        return std::unexpected(LibraryError::LibraryCloseFailure);
    }

    return symbol;
}

auto close_library(LibraryHandle handle) -> std::expected<void, LibraryError> {
    int32_t result = dlclose(handle);
    if (result != 0) {
        core::error(dlerror());
        return std::unexpected(LibraryError::LibraryCloseFailure);
    }

    return {};
}

void invoke_signal(Signal signal) {
    switch (signal) {
        case Signal::DebugTrap:
            raise(SIGTRAP);
            break;
    }
}

auto has_elevated_privileges() -> bool { return geteuid() == 0; }

} // namespace muon
