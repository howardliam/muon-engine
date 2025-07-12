#pragma once

#include <cstdint>

#if INTPTR_MAX == INT64_MAX

#ifdef __linux__
#define MU_PLATFORM_LINUX

#elifdef _WIN32
#define MU_PLATFORM_WINDOWS

#endif

#else
#error "32 bit systems are not supported"

#endif
