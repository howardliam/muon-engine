#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace muon {

class buffer {
public:
    using value_type = std::uint8_t;
    using size_type = std::size_t;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = value_type *;
    using const_iterator = const value_type *;

    buffer() = delete;
    buffer(size_type size) noexcept;
    buffer(pointer data, size_type size) noexcept;
    buffer(std::string_view text) noexcept;
    buffer(const buffer &other) noexcept;

    ~buffer() noexcept;

    auto data() noexcept -> pointer;
    auto data() const noexcept -> const_pointer;

    auto begin() noexcept -> iterator;
    auto begin() const noexcept -> const_iterator;

    auto end() noexcept -> iterator;
    auto end() const noexcept -> const_iterator;

    auto size() const noexcept -> size_type;

    template <typename T>
    auto as() -> T * {
        return reinterpret_cast<T *>(data_);
    }

    template <typename T>
    auto as() const -> const T * {
        return reinterpret_cast<const T *>(data_);
    }

private:
    void allocate();

private:
    pointer data_{nullptr};
    size_type size_{0};
};

auto operator==(const buffer &lhs, const buffer &rhs) noexcept -> bool;

class buffer_view {
public:
    using value_type = std::uint8_t;
    using size_type = std::size_t;
    using const_pointer = const value_type *;
    using const_iterator = const value_type *;

    buffer_view() = delete;
    buffer_view(const buffer &buffer) noexcept;
    buffer_view(const buffer_view &other) noexcept;

    auto data() const noexcept -> const_pointer;

    auto begin() const noexcept -> const_iterator;

    auto end() const noexcept -> const_iterator;

    auto size() const noexcept -> size_type;

    template <typename T>
    auto as() const -> const T * {
        return reinterpret_cast<const T *>(data_);
    }

private:
    const_pointer data_{nullptr};
    size_type size_{0};
};

auto operator==(const buffer_view &lhs, const buffer_view &rhs) noexcept -> bool;

}
