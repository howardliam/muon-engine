#pragma once

#include "fmt/base.h"
#include "muon/core/debug.hpp"
#include "muon/core/log.hpp"
#include "muon/utils/platform.hpp"

#include <concepts>
#include <utility>

namespace muon {

template <typename T>
concept boolean_testable = requires(T &&t) {
    { static_cast<bool>(std::forward<T>(t)) } -> std::same_as<bool>;
};

namespace core {

template <boolean_testable Condition, typename... Args>
void expect(Condition &&condition, fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (debug_enabled) {
        if (!static_cast<bool>(std::forward<Condition>(condition))) {
            log::internal::s_coreLogger->error(message, std::forward<Args>(args)...);
            utils::invokeDebugTrap();
        }
    }
}

} // namespace core

namespace client {

template <boolean_testable Condition, typename... Args>
void expect(Condition &&condition, fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (debug_enabled) {
        if (!static_cast<bool>(std::forward<Condition>(condition))) {
            log::internal::s_clientLogger->error(message, std::forward<Args>(args)...);
            utils::invokeDebugTrap();
        }
    }
}

} // namespace client

} // namespace muon
