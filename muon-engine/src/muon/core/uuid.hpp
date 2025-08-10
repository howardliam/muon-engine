#pragma once

#include "fmt/base.h"

#include <array>
#include <compare>
#include <cstdint>
#include <functional>
#include <string>

namespace muon {

class uuid {
public:
    using value_type = std::array<uint8_t, 16>;
    using pointer = value_type::pointer;
    using const_pointer = value_type::const_pointer;
    using iterator = value_type::iterator;
    using const_iterator = value_type::const_iterator;
    using size_type = value_type::size_type;
    enum version_type {
        version_unknown,
        version_random_number_based = 4,
        version_unix_time_based = 7,
    };

    auto version() const noexcept -> version_type;

    auto data() noexcept -> pointer;
    auto data() const noexcept -> const_pointer;

    auto begin() noexcept -> iterator;
    auto begin() const noexcept -> const_iterator;

    auto end() noexcept -> iterator;
    auto end() const noexcept -> const_iterator;

    auto size() const noexcept -> size_type;

    auto is_nil() const noexcept -> bool;

    auto to_string() const -> std::string;

    friend auto operator==(const uuid &lhs, const uuid &rhs) noexcept -> bool;
    friend auto operator<=>(const uuid &lhs, const uuid &rhs) noexcept -> std::strong_ordering;

private:
    value_type bytes_{};
};

auto operator==(const uuid &lhs, const uuid &rhs) noexcept -> bool;
auto operator<=>(const uuid &lhs, const uuid &rhs) noexcept -> std::strong_ordering;

class uuid4_generator {
public:
    uuid4_generator() = default;

    auto operator()() -> uuid;
};

class uuid7_generator {
public:
    uuid7_generator() = default;

    auto operator()() -> uuid;
};

} // namespace muon

template<>
struct std::hash<muon::uuid> {
    auto operator()(const muon::uuid &uuid) const -> size_t;
};

template <>
struct fmt::formatter<muon::uuid> : formatter<string_view> {
    auto format(const muon::uuid &uuid, format_context &ctx) const -> format_context::iterator;
};
