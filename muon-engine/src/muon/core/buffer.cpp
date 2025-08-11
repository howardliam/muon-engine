#include "muon/core/buffer.hpp"

#include <cstdlib>
#include <cstring>

namespace muon {

Buffer::Buffer(SizeType size) noexcept : size_{size} { allocate(); }

Buffer::Buffer(Pointer data, SizeType size) noexcept : size_{size} {
    allocate();
    std::memcpy(data_, data, size_);
}

Buffer::Buffer(std::string_view text) noexcept : size_{text.size()} {
    allocate();
    std::memcpy(data_, text.data(), size_);
}

Buffer::Buffer(const Buffer &other) noexcept : size_{other.size()} {
    allocate();
    std::memcpy(data_, other.data(), size_);
}

Buffer::~Buffer() noexcept {
    free(data_);
}

auto Buffer::data() noexcept -> Pointer { return data_; }
auto Buffer::data() const noexcept -> ConstPointer { return data_; }

auto Buffer::begin() noexcept -> Iterator { return data_; }
auto Buffer::begin() const noexcept -> ConstIterator { return data_; }

auto Buffer::end() noexcept -> Iterator { return data_ + size_; }
auto Buffer::end() const noexcept -> ConstIterator { return data_ + size_; }

auto Buffer::size() const noexcept -> SizeType { return size_; }

void Buffer::allocate() {
    data_ = static_cast<Pointer>(calloc(size_, sizeof(ValueType)));
}

auto Buffer::operator==(const Buffer &rhs) const noexcept -> bool {
    if (size() != rhs.size()) {
        return false;
    }
    return std::memcmp(data(), rhs.data(), size()) == 0;
}

BufferView::BufferView(const Buffer &buffer) noexcept : data_{buffer.data()}, size_{buffer.size()} {}

BufferView::BufferView(const BufferView &other) noexcept : data_{other.data()}, size_{other.size()} {}

auto BufferView::data() const noexcept -> ConstPointer { return data_; }

auto BufferView::begin() const noexcept -> ConstIterator { return data_; }

auto BufferView::end() const noexcept -> ConstIterator { return data_ + size_; }

auto BufferView::size() const noexcept -> SizeType { return size_; }

auto BufferView::operator==(const BufferView &rhs) const noexcept -> bool {
    if (size() != rhs.size()) {
        return false;
    }
    return std::memcmp(data(), rhs.data(), size()) == 0;
}

}
