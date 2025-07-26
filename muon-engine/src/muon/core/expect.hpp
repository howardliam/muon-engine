#pragma once

#include "fmt/base.h"
#include "muon/core/log.hpp"
#include "muon/core/signals.hpp"

#include <concepts>
#include <utility>

namespace muon {

template <typename T>
concept BooleanTestable = requires(T &&t) {
    { static_cast<bool>(std::forward<T>(t)) } -> std::same_as<bool>;
};

namespace core {

template <BooleanTestable Condition, typename... Args>
auto expect(Condition &&condition, fmt::format_string<Args...> message, Args &&...args) -> void {
    if (!static_cast<bool>(std::forward<Condition>(condition))) {
        core::error(message, args...);
        debugBreak();
    }
}

} // namespace core

namespace client {

template <BooleanTestable Condition, typename... Args>
auto expect(Condition &&condition, fmt::format_string<Args...> message, Args &&...args) -> void {
    if (!static_cast<bool>(std::forward<Condition>(condition))) {
        client::error(message, args...);
        debugBreak();
    }
}

} // namespace client

} // namespace muon
