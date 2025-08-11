#pragma once

#include "fmt/base.h"

#include <array>
#include <compare>
#include <cstdint>
#include <functional>
#include <string>

namespace muon {

enum class UuidVersion {
    Unknown,
    RandomNumber = 4,
    UnixTime =7,
};

class Uuid {
public:
    using ValueType = std::array<uint8_t, 16>;
    using Pointer = ValueType::pointer;
    using ConstPointer = ValueType::const_pointer;
    using Iterator = ValueType::iterator;
    using ConstIterator = ValueType::const_iterator;
    using SizeType = ValueType::size_type;

    static auto uuid4() noexcept -> Uuid;
    static auto uuid7() noexcept -> Uuid;

    auto data() noexcept -> Pointer;
    auto data() const noexcept -> ConstPointer;

    auto size() const noexcept -> SizeType;

    auto version() const noexcept -> UuidVersion;
    auto is_nil() const noexcept -> bool;

    auto to_string() const -> std::string;

    auto begin() noexcept -> Iterator;
    auto begin() const noexcept -> ConstPointer;

    auto end() noexcept -> Iterator;
    auto end() const noexcept -> ConstPointer;

    auto operator==(const Uuid &rhs) noexcept -> bool;
    auto operator<=>(const Uuid &rhs) noexcept -> std::strong_ordering;

private:
    ValueType data_{};
};

} // namespace muon

template<>
struct std::hash<muon::Uuid> {
    auto operator()(const muon::Uuid &uuid) const -> size_t;
};

template <>
struct fmt::formatter<muon::Uuid> : formatter<string_view> {
    auto format(const muon::Uuid &uuid, format_context &ctx) const -> format_context::iterator;
};
