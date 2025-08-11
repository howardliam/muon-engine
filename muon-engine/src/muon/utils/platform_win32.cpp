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


auto open_library(const std::filesystem::path &path) -> std::expected<LibraryHandle, LibraryError> {
    LibraryHandle handle = LoadLibrary(path.c_str());
    if (!handle) {
        print_error(GetLastError());
        return std::unexpected(LibraryError::LibraryOpenFailure);
    }

    return handle;
}

auto load_symbol(LibraryHandle handle, std::string_view name) -> std::expected<SymbolHandle, LibraryError> {
    SymbolHandle symbol = GetProcAddress(handle, name.data());
    if (!symbol) {
        print_error(GetLastError());
        return std::unexpected(LibraryError::SymbolLoadFailure);
    }

    return symbol;
}

auto close_library(LibraryHandle handle) -> std::expected<void, LibraryError> {
    int32_t result = FreeLibrary(handle);
    if (result != 0) {
        print_error(GetLastError());
        return std::unexpected(LibraryError::LibraryCloseFailure);
    }

    return {};
}

void invoke_signal(Signal signal) {
    switch (signal) {
        case Signal::DebugTrap:
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
