#include "muon/utils/platform.hpp"

#include "muon/core/log.hpp"

#include <aclapi.h>
#include <windows.h>

namespace muon::utils {

void printError(const DWORD errorCode) {
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


auto openDynamicLibrary(const std::filesystem::path &path) -> std::expected<void *, DynamicLibraryError> {
    void *handle = LoadLibrary(path.c_str());
    if (!handle) {
        printError(GetLastError());
        return std::unexpected(DynamicLibraryError::LibraryOpenFailure);
    }

    return handle;
}

auto loadSymbol(void *handle, const std::string_view name) -> std::expected<void *, DynamicLibraryError> {
    void *symbol = GetProcAddress(handle, name.data());
    if (!symbol) {
        printError(GetLastError());
        return std::unexpected(DynamicLibraryError::SymbolLoadFailure);
    }

    return symbol;
}

auto closeDynamicLibrary(void *handle) -> std::expected<void, DynamicLibraryError> {
    int32_t result = FreeLibrary(handle);
    if (result != 0) {
        printError(GetLastError());
        return std::unexpected(DynamicLibraryError::LibraryCloseFailure);
    }

    return {};
}

void invokeDebugTrap() { __debugbreak(); }

auto isRunAsRoot() -> bool {
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
