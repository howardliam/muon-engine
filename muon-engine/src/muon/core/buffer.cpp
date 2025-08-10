#include "muon/core/buffer.hpp"

#include <cstdlib>
#include <cstring>

namespace muon {

buffer::buffer(size_type size) noexcept : size_{size} { allocate(); }

buffer::buffer(pointer data, size_type size) noexcept : size_{size} {
    allocate();
    std::memcpy(data_, data, size_);
}

buffer::buffer(std::string_view text) noexcept : size_{text.size()} {
    allocate();
    std::memcpy(data_, text.data(), size_);
}

buffer::buffer(const buffer &other) noexcept : size_{other.size()} {
    allocate();
    std::memcpy(data_, other.data(), size_);
}

buffer::~buffer() noexcept {
    free(data_);
}

auto buffer::data() noexcept -> pointer { return data_; }
auto buffer::data() const noexcept -> const_pointer { return data_; }

auto buffer::begin() noexcept -> iterator { return data_; }
auto buffer::begin() const noexcept -> const_iterator { return data_; }

auto buffer::end() noexcept -> iterator { return data_ + size_; }
auto buffer::end() const noexcept -> const_iterator { return data_ + size_; }

auto buffer::size() const noexcept -> size_type { return size_; }

void buffer::allocate() {
    data_ = static_cast<pointer>(calloc(size_, sizeof(value_type)));
}

auto operator==(const buffer &lhs, const buffer &rhs) noexcept -> bool {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    return std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
}

buffer_view::buffer_view(const buffer &buffer) noexcept : data_{buffer.data()}, size_{buffer.size()} {}

buffer_view::buffer_view(const buffer_view &other) noexcept : data_{other.data()}, size_{other.size()} {}

auto buffer_view::data() const noexcept -> const_pointer { return data_; }

auto buffer_view::begin() const noexcept -> const_iterator { return data_; }

auto buffer_view::end() const noexcept -> const_iterator { return data_ + size_; }

auto buffer_view::size() const noexcept -> size_type { return size_; }

auto operator==(const buffer_view &lhs, const buffer_view &rhs) noexcept -> bool {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    return std::memcmp(lhs.data(), rhs.data(), lhs.size()) == 0;
}

}
