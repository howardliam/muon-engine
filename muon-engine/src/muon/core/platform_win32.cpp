#include "muon/core/log.hpp"
#include "muon/core/platform.hpp"

#include <aclapi.h>
#include <windows.h>

namespace muon {

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
