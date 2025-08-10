#pragma once

#include "fmt/base.h"

#include <array>
#include <compare>
#include <cstdint>
#include <functional>
#include <string>

namespace muon {

struct uuid {
    using data_type = std::array<uint8_t, 16>;
    using iterator = data_type::iterator;
    using const_iterator = data_type::const_iterator;
    using size_type = data_type::size_type;

    data_type data{};

    auto begin() noexcept -> iterator;
    auto begin() const noexcept -> const_iterator;

    auto end() noexcept -> iterator;
    auto end() const noexcept -> const_iterator;

    auto size() const noexcept -> size_type;

    auto is_nil() const noexcept -> bool;

    auto to_string() const -> std::string;
};

auto operator==(const uuid &lhs, const uuid &rhs) noexcept -> bool;
auto operator<=>(const uuid &lhs, const uuid &rhs) noexcept -> std::strong_ordering;

class uuid4_generator {
public:
    uuid4_generator();

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
