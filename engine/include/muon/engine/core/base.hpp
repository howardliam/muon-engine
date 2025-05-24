#pragma once

#define MU_EXPAND_MACRO(x) x
#define MU_STRINGIFY_MACRO(x) #x

#ifdef MU_DEBUG_ENABLED

    #ifdef _WIN32

        #include <windows.h>
        #define MU_DEBUG_BREAK() __debugbreak()

    #else

        #include <csignal>
        #define MU_DEBUG_BREAK() raise(SIGTRAP)

    #endif


#else

    #define MU_DEBUG_BREAK()

#endif
