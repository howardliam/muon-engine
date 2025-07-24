#pragma once

#include "muon/core/log.hpp"
#include "muon/core/signals.hpp"

#include <string_view>

namespace muon {

namespace core {

template <typename... Args>
constexpr inline auto expect(bool condition, std::string_view message, Args &&...args) -> void {
    if (!condition) {
        core::error(message, args...);
        debugBreak();
    }
}

} // namespace core

namespace client {

template <typename... Args>
constexpr inline auto expect(bool condition, std::string_view message, Args &&...args) -> void {
    if (!condition) {
        client::error(message, args...);
        debugBreak();
    }
}

} // namespace client

} // namespace muon
