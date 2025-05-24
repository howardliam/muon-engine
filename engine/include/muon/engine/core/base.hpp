#pragma once

#define MU_EXPAND_MACRO(x) x
#define MU_STRINGIFY_MACRO(x) #x

#ifndef NDEBUG

    #include <csignal>
    #define MU_DEBUG_EXIT() raise(SIGTRAP)

#else

    #define MU_DEBUG_EXIT()

#endif
