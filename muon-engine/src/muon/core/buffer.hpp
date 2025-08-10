#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>

namespace muon {

class raw_buffer {
public:
    using value_type = std::uint8_t;
    using size_type = std::size_t;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = value_type *;
    using const_iterator = const value_type *;

    raw_buffer() = default;
    raw_buffer(size_t size);
    raw_buffer(const raw_buffer &other);

    void allocate(size_t size);
    void release();

    auto data() noexcept -> pointer;
    auto data() const noexcept -> const_pointer;

    auto begin() noexcept -> iterator;
    auto begin() const noexcept -> const_iterator;

    auto end() noexcept -> iterator;
    auto end() const noexcept -> const_iterator;

    constexpr auto size() const noexcept -> size_type;

    template <typename T>
    auto as() -> T * {
        return reinterpret_cast<T *>(data_);
    }

private:
    value_type *data_{nullptr};
    size_type size_{0};
};

auto operator==(const raw_buffer &lhs, const raw_buffer &rhs) noexcept -> bool;
auto operator<=>(const raw_buffer &lhs, const raw_buffer &rhs) noexcept -> std::strong_ordering;

class buffer final : public raw_buffer {
public:
    buffer() = default;
    buffer(size_t size);
    buffer(const buffer &other);

    ~buffer();
};

}
