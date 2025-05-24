#pragma once

#include "muon/engine/core/log.hpp"
#include "muon/engine/core/base.hpp"

// implementation borrowed from https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Core/Assert.h

#ifndef NDEBUG

    #define MU_INTERNAL_ASSERT_IMPL(type, check, msg, ...) \
        { if(!(check)) { MU##type##ERROR(msg, __VA_ARGS__); MU_DEBUG_EXIT(); } }

    #define MU_INTERNAL_ASSERT_WITH_MSG(type, check, ...) \
        MU_INTERNAL_ASSERT_IMPL(type, check, "assertion failed: {0}", __VA_ARGS__)

    #define MU_INTERNAL_ASSERT_NO_MSG(type, check) \
        MU_INTERNAL_ASSERT_IMPL(type, check, "assertion '{0}' failed at {1}:{2}", MU_STRINGIFY_MACRO(check), __FILE__, __LINE__)

    #define MU_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro

    #define MU_INTERNAL_ASSERT_GET_MACRO(...) \
        MU_EXPAND_MACRO( MU_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, MU_INTERNAL_ASSERT_WITH_MSG, MU_INTERNAL_ASSERT_NO_MSG) )

    #define MU_CORE_ASSERT(...) MU_EXPAND_MACRO( MU_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
    #define MU_ASSERT(...) MU_EXPAND_MACRO( MU_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )

#else

    #define MU_CORE_ASSERT(...)
    #define MU_ASSERT(...)

#endif
