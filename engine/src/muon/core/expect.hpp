#pragma once

#include "fmt/base.h"
#include "muon/core/log.hpp"
#include "muon/core/signals.hpp"

namespace muon {

namespace core {

template <typename... Args>
auto expect(bool condition, fmt::format_string<Args...> message, Args &&...args) -> void {
    if (!condition) {
        core::error(message, args...);
        debugBreak();
    }
}

} // namespace core

namespace client {

template <typename... Args>
auto expect(bool condition, fmt::format_string<Args...> message, Args &&...args) -> void {
    if (!condition) {
        client::error(message, args...);
        debugBreak();
    }
}

} // namespace client

} // namespace muon
