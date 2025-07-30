#include "muon/core/log.hpp"
#include "muon/core/platform.hpp"

#include <aclapi.h>
#include <sddl.h>
#include <windows.h>

namespace muon {

auto invokeDebugTrap() -> void { __debugbreak(); }

auto determineProcessElevation() -> bool {
    auto cleanUp = [](HANDLE token) {
        if (token) {
            CloseHandle(token);
        }
    };

    BOOL isElevated = FALSE;
    HANDLE token = NULL;
    TOKEN_ELEVATION elevation;
    DWORD dwSize;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        core::error("failed to get process token: {}", GetLastError());
        cleanUp(token);
        return false;
    }

    if (!GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
        core::error("failed to get token information: {}", GetLastError());
        cleanUp(token);
        return false;
    }

    isElevated = elevation.TokenIsElevated;

    return static_cast<bool>(isElevated);
}

} // namespace muon
