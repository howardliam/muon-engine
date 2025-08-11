#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace muon {

class Buffer {
public:
    using ValueType = std::uint8_t;
    using SizeType = std::size_t;
    using Pointer = ValueType *;
    using ConstPointer = const ValueType *;
    using Iterator = ValueType *;
    using ConstIterator = const ValueType *;

    Buffer() = delete;
    Buffer(SizeType size) noexcept;
    Buffer(Pointer data, SizeType size) noexcept;
    Buffer(std::string_view text) noexcept;
    Buffer(const Buffer &other) noexcept;

    ~Buffer() noexcept;

    auto data() noexcept -> Pointer;
    auto data() const noexcept -> ConstPointer;

    auto begin() noexcept -> Iterator;
    auto begin() const noexcept -> ConstIterator;

    auto end() noexcept -> Iterator;
    auto end() const noexcept -> ConstIterator;

    auto size() const noexcept -> SizeType;

    template <typename T>
    auto as() -> T * {
        return reinterpret_cast<T *>(data_);
    }

    template <typename T>
    auto as() const -> const T * {
        return reinterpret_cast<const T *>(data_);
    }

    auto operator==(const Buffer &rhs) const noexcept -> bool;

private:
    void allocate();

private:
    Pointer data_{nullptr};
    SizeType size_{0};
};


class BufferView {
public:
    using ValueType = std::uint8_t;
    using SizeType = std::size_t;
    using ConstPointer = const ValueType *;
    using ConstIterator = const ValueType *;

    BufferView() = delete;
    BufferView(const Buffer &buffer) noexcept;
    BufferView(const BufferView &other) noexcept;

    auto data() const noexcept -> ConstPointer;

    auto begin() const noexcept -> ConstIterator;

    auto end() const noexcept -> ConstIterator;

    auto size() const noexcept -> SizeType;

    template <typename T>
    auto as() const -> const T * {
        return reinterpret_cast<const T *>(data_);
    }

    auto operator==(const BufferView &rhs) const noexcept -> bool;

private:
    ConstPointer data_{nullptr};
    SizeType size_{0};
};

}
