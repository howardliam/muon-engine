#include "muon/utils/platform.hpp"

#include "muon/core/log.hpp"

#include <csignal>
#include <dlfcn.h>
#include <unistd.h>

namespace muon::utils {

auto openDynamicLibrary(const std::filesystem::path &path) -> std::expected<void *, DynamicLibraryError> {
    void *handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        core::error(dlerror());
        return std::unexpected(DynamicLibraryError::LibraryOpenFailure);
    }

    return handle;
}

auto loadSymbol(void *handle, const std::string_view name) -> std::expected<void *, DynamicLibraryError> {
    void *symbol = dlsym(handle, name.data());
    if (!symbol) {
        core::error(dlerror());
        return std::unexpected(DynamicLibraryError::SymbolLoadFailure);
    }

    return symbol;
}

auto closeDynamicLibrary(void *handle) -> std::expected<void, DynamicLibraryError> {
    int32_t result = dlclose(handle);
    if (result != 0) {
        core::error(dlerror());
        return std::unexpected(DynamicLibraryError::LibraryCloseFailure);
    }

    return {};
}

void invokeDebugTrap() { raise(SIGTRAP); }

auto isRunAsRoot() -> bool { return geteuid() == 0; }

} // namespace muon
