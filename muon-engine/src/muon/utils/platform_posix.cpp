#include "muon/utils/platform.hpp"

#include "muon/core/log.hpp"

#include <csignal>
#include <dlfcn.h>
#include <unistd.h>

namespace muon::utils {

auto open_library(const std::filesystem::path &path) -> std::expected<library_handle, library_error> {
    library_handle handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        core::error(dlerror());
        return std::unexpected(library_error::library_open_failure);
    }

    return handle;
}

auto load_symbol(library_handle handle, const std::string_view name) -> std::expected<symbol_handle, library_error> {
    symbol_handle symbol = dlsym(handle, name.data());
    if (!symbol) {
        core::error(dlerror());
        return std::unexpected(library_error::symbol_load_failure);
    }

    return symbol;
}

auto close_library(library_handle handle) -> std::expected<void, library_error> {
    int32_t result = dlclose(handle);
    if (result != 0) {
        core::error(dlerror());
        return std::unexpected(library_error::library_close_failure);
    }

    return {};
}

void invoke_signal(signal signal) {
    switch (signal) {
        case signal::debug_trap:
            raise(SIGTRAP);
            break;
    }
}

auto has_elevated_privileges() -> bool { return geteuid() == 0; }

} // namespace muon
