#pragma once

#include <cstdint>

namespace muon {

class RawBuffer {
public:
    RawBuffer() = default;
    RawBuffer(uint64_t size);
    RawBuffer(const RawBuffer &other);

    void allocate(uint64_t size);
    void release();

public:
    auto getData() -> uint8_t *;
    auto getSize() -> uint64_t;

    template <typename T>
    auto as() -> T * {
        return reinterpret_cast<T *>(m_data);
    }

private:
    uint8_t *m_data{nullptr};
    uint64_t m_size{0};
};

class Buffer : public RawBuffer {
public:
    Buffer() = default;
    Buffer(uint64_t size);
    Buffer(const RawBuffer &other);

    ~Buffer();
};

}
