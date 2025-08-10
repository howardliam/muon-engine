#include "muon/core/uuid.hpp"

#include "fmt/format.h"
#include "sodium/randombytes.h"
#include <algorithm>
#include <cstdint>

namespace muon {

auto uuid::begin() noexcept -> iterator { return data.begin(); }
auto uuid::begin() const noexcept -> const_iterator { return data.begin(); }

auto uuid::end() noexcept -> iterator { return data.end(); }
auto uuid::end() const noexcept -> const_iterator { return data.end(); }

auto uuid::size() const noexcept -> size_type { return data.size(); }

auto uuid::to_string() const -> std::string {
    return fmt::format(
        "{:02x}{:02x}{:02x}{:02x}-"
        "{:02x}{:02x}-"
        "{:02x}{:02x}-"
        "{:02x}{:02x}-"
        "{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
        data[0], data[1], data[2], data[3],
        data[4], data[5],
        data[6], data[7],
        data[8], data[9],
        data[10], data[11], data[12], data[13], data[14], data[15]
    );
}

auto uuid::is_nil() const noexcept -> bool {
    return std::all_of(data.begin(), data.end(), [](uint8_t byte) { return byte == 0; });
}

auto operator==(const uuid &lhs, const uuid &rhs) noexcept -> bool {
    return lhs.data == rhs.data;
}

auto operator<=>(const uuid &lhs, const uuid &rhs) noexcept -> std::strong_ordering {
    return lhs.data <=> rhs.data;
}

uuid4_generator::uuid4_generator() {}

auto uuid4_generator::operator()() -> uuid {
    uuid uuid;

    randombytes_buf(uuid.begin(), uuid.size());
    *(uuid.begin() + 6) &= 0x0f;
    *(uuid.begin() + 6) |= 0x40;

    *(uuid.begin() + 8) &= 0x3f;
    *(uuid.begin() + 8) |= 0x80;

    return uuid;
}

}

namespace std {

auto hash<muon::uuid>::operator()(const muon::uuid &uuid) const -> size_t {
    const uint64_t *ptr = reinterpret_cast<const uint64_t *>(uuid.data.data());
    size_t hash1 = std::hash<uint64_t>{}(ptr[0]);
    size_t hash2 = std::hash<uint64_t>{}(ptr[1]);
    return hash1 ^ (hash2 << 1);
}

} // namespace std
