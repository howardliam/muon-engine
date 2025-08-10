#include "muon/core/buffer.hpp"

#include <cstring>

namespace muon {

raw_buffer::raw_buffer(size_t size) { allocate(size); }
raw_buffer::raw_buffer(const raw_buffer &other) {
    allocate(other.size_);
    std::memcpy(data_, other.data_, size_);
}

void raw_buffer::allocate(size_t size) {
    release();

    size_ = size;
    data_ = new value_type[size_];
}

void raw_buffer::release() {
    delete[] data_;
    data_ = nullptr;
    size_ = 0;
}

auto raw_buffer::data() noexcept -> pointer { return data_; }
auto raw_buffer::data() const noexcept -> const_pointer { return data_; }

auto raw_buffer::begin() noexcept -> iterator { return data_; }
auto raw_buffer::begin() const noexcept -> const_iterator { return data_; }

auto raw_buffer::end() noexcept -> iterator { return data_ + size_; }
auto raw_buffer::end() const noexcept -> const_iterator { return data_ + size_; }

auto raw_buffer::size() const noexcept -> size_type { return size_; }

auto operator==(const raw_buffer &lhs, const raw_buffer &rhs) noexcept -> bool {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    return std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
}

buffer::buffer(size_t size) : raw_buffer{size} {}

buffer::buffer(const buffer &other) : raw_buffer{other} {}

buffer::~buffer() { release(); }

}
