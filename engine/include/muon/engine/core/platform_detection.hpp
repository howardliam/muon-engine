#pragma once

#include <cstdint>

#ifdef __linux__

    #if INTPTR_MAX == INT64_MAX
        #define MU_PLATFORM_LINUX
    #else
        #error "32 bit Linux is not supported"
    #endif

#elifdef _WIN32

        #ifdef _WIN64
            #define MU_PLATFORM_WINDOWS
        #else
            #error "32 bit Windows is not supported"
        #endif

#else

    #error "Unknown platform"

#endif
