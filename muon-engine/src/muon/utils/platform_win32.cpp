#include "muon/utils/platform.hpp"

#include "muon/core/log.hpp"

#include <aclapi.h>
#include <windows.h>

namespace muon::utils {

void print_error(const DWORD errorCode) {
    void *msgBuffer;
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<char *>(&msgBuffer),
        0,
        nullptr
    );

    core::error("{}", static_cast<char *>(msgBuffer));

    LocalFree(msgBuffer);
}


auto open_library(const std::filesystem::path &path) -> std::expected<library_handle, library_error> {
    void *handle = LoadLibrary(path.c_str());
    if (!handle) {
        print_error(GetLastError());
        return std::unexpected(library_error::library_open_failure);
    }

    return handle;
}

auto load_symbol(library_handle handle, std::string_view name) -> std::expected<symbol_handle, library_error> {
    void *symbol = GetProcAddress(handle, name.data());
    if (!symbol) {
        print_error(GetLastError());
        return std::unexpected(library_error::symbol_load_failure);
    }

    return symbol;
}

auto close_library(library_handle handle) -> std::expected<void, library_error> {
    int32_t result = FreeLibrary(handle);
    if (result != 0) {
        print_error(GetLastError());
        return std::unexpected(library_error::library_close_failure);
    }

    return {};
}

void invoke_signal(signal signal) {
    switch (signal) {
        case signal::debug_trap:
            __debugbreak();
            break;
    }
}

auto has_elevated_privileges() -> bool {
    BOOL isAdmin = FALSE;
    PSID administratorsGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_GROUP;

    auto result = AllocateAndInitializeSid(
        &ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &administratorsGroup
    );

    if (result) {
        CheckTokenMembership(nullptr, administratorsGroup, &isAdmin);
        FreeSid(administratorsGroup);
    }

    return static_cast<bool>(isAdmin);
}

} // namespace muon
