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

constexpr auto uuid::size() const noexcept -> size_type { return data.size(); }

auto uuid::to_string() const -> std::string {
    const auto part1 = fmt::format("{:02x}{:02x}{:02x}{:02x}", data[0], data[1], data[2], data[3]);
    const auto part2 = fmt::format("{:02x}{:02x}", data[4], data[5]);
    const auto part3 = fmt::format("{:02x}{:02x}", data[6], data[7]);
    const auto part4 = fmt::format("{:02x}{:02x}", data[8], data[9]);
    const auto part5 = fmt::format("{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}", data[10], data[11], data[12], data[13], data[14], data[15]);

    return fmt::format("{}-{}-{}-{}-{}", part1, part2, part3, part4, part5);
}

auto uuid::operator==(const uuid &rhs) noexcept -> bool {
    return data == rhs.data;
}

auto uuid::operator<=>(const uuid &rhs) noexcept -> std::strong_ordering {
    return data <=> rhs.data;
}

auto uuid::is_nil() const noexcept -> bool {
    return std::all_of(data.begin(), data.end(), [](uint8_t byte) { return byte == 0; });
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
