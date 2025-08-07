#include "muon/core/buffer.hpp"

#include <cstring>

namespace muon {

RawBuffer::RawBuffer(uint64_t size) { allocate(size); }

RawBuffer::RawBuffer(const RawBuffer &other) {
    allocate(other.m_size);
    std::memcpy(m_data, other.m_data, m_size);
}

void RawBuffer::allocate(uint64_t size) {
    release();

    m_size = size;
    m_data = new uint8_t[m_size];
}

void RawBuffer::release() {
    delete[] m_data;
    m_data = nullptr;
    m_size = 0;
}

auto RawBuffer::getData() -> uint8_t * { return m_data; }

auto RawBuffer::getSize() -> uint64_t { return m_size; }

Buffer::Buffer(uint64_t size) : RawBuffer{size} {}

Buffer::Buffer(const RawBuffer &other) : RawBuffer{other} {}

Buffer::~Buffer() { release(); }

}
