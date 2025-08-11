#pragma once

#include "fmt/base.h"
#include "muon/core/debug.hpp"
#include "muon/core/log.hpp"
#include "muon/utils/platform.hpp"

#include <concepts>
#include <utility>

namespace muon {

template <typename T>
concept BooleanTestable = requires(T &&t) {
    { static_cast<bool>(std::forward<T>(t)) } -> std::same_as<bool>;
};

namespace core {

template <BooleanTestable Condition, typename... Args>
void expect(Condition &&condition, fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (DEBUG_ENABLED) {
        if (!static_cast<bool>(std::forward<Condition>(condition))) {
            log::internal::core_logger->error(message, std::forward<Args>(args)...);
            utils::invoke_signal(utils::Signal::DebugTrap);
        }
    }
}

} // namespace core

namespace client {

template <BooleanTestable Condition, typename... Args>
void expect(Condition &&condition, fmt::format_string<Args...> message, Args &&...args) {
    if constexpr (DEBUG_ENABLED) {
        if (!static_cast<bool>(std::forward<Condition>(condition))) {
            log::internal::client_logger->error(message, std::forward<Args>(args)...);
            utils::invoke_signal(utils::Signal::DebugTrap);
        }
    }
}

} // namespace client

} // namespace muon
