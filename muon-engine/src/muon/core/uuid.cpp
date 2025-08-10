#include "muon/core/uuid.hpp"

#include "fmt/format.h"
#include "sodium/randombytes.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>

namespace muon {

auto uuid::version() const noexcept -> version_type {
    const auto &octet9 = bytes_[6];
    if ((octet9 & 0xf0) == 0x40) {
        return version_random_number_based;
    } else if ((octet9 & 0xf0) == 0x70) {
        return version_unix_time_based;
    } else {
        return version_unknown;
    }
}

auto uuid::data() noexcept -> pointer { return bytes_.data(); }
auto uuid::data() const noexcept -> const_pointer { return bytes_.data(); }

auto uuid::begin() noexcept -> iterator { return bytes_.begin(); }
auto uuid::begin() const noexcept -> const_iterator { return bytes_.begin(); }

auto uuid::end() noexcept -> iterator { return bytes_.end(); }
auto uuid::end() const noexcept -> const_iterator { return bytes_.end(); }

auto uuid::size() const noexcept -> size_type { return bytes_.size(); }

auto uuid::to_string() const -> std::string {
    return fmt::format(
        "{:02x}{:02x}{:02x}{:02x}-"
        "{:02x}{:02x}-"
        "{:02x}{:02x}-"
        "{:02x}{:02x}-"
        "{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
        bytes_[0], bytes_[1], bytes_[2], bytes_[3],
        bytes_[4], bytes_[5],
        bytes_[6], bytes_[7],
        bytes_[8], bytes_[9],
        bytes_[10], bytes_[11], bytes_[12], bytes_[13], bytes_[14], bytes_[15]
    );
}

auto uuid::is_nil() const noexcept -> bool {
    return std::all_of(bytes_.begin(), bytes_.end(), [](uint8_t byte) { return byte == 0; });
}

auto operator==(const uuid &lhs, const uuid &rhs) noexcept -> bool {
    return lhs.bytes_ == rhs.bytes_;
}

auto operator<=>(const uuid &lhs, const uuid &rhs) noexcept -> std::strong_ordering {
    return lhs.bytes_ <=> rhs.bytes_;
}

auto uuid4_generator::operator()() -> uuid {
    uuid uuid;

    randombytes_buf(uuid.begin(), uuid.size());
    *(uuid.begin() + 6) &= 0x0f;
    *(uuid.begin() + 6) |= 0x40;

    *(uuid.begin() + 8) &= 0x3f;
    *(uuid.begin() + 8) |= 0x80;

    return uuid;
}

auto uuid7_generator::operator()() -> uuid {
    uuid uuid;

    auto now = std::chrono::system_clock::now();
    auto epoch_millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    for (size_t i = 0; i < 6; i++) {
        uuid.data()[i] = static_cast<uint8_t>((epoch_millis >> ((5 - i) * 8)) * 0xff);
    }

    randombytes_buf(uuid.begin() + 6, 2);
    randombytes_buf(uuid.begin() + 8, 8);
    *(uuid.begin() + 6) &= 0x0f;
    *(uuid.begin() + 6) |= 0x70;

    *(uuid.begin() + 8) &= 0x3f;
    *(uuid.begin() + 8) |= 0x80;

    return uuid;
}

}

auto std::hash<muon::uuid>::operator()(const muon::uuid &uuid) const -> size_t {
    const uint64_t *ptr = reinterpret_cast<const uint64_t *>(uuid.data());
    size_t hash1 = std::hash<uint64_t>{}(ptr[0]);
    size_t hash2 = std::hash<uint64_t>{}(ptr[1]);
    return hash1 ^ (hash2 << 1);
}

auto fmt::formatter<muon::uuid>::format(const muon::uuid &uuid, format_context &ctx) const -> format_context::iterator {
    return formatter<string_view>::format(uuid.to_string(), ctx);
}
