#include "muon/core/uuid.hpp"

#include "fmt/format.h"
#include "sodium/randombytes.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>

namespace muon {

auto Uuid::uuid4() noexcept -> Uuid {
    Uuid uuid;

    randombytes_buf(uuid.begin(), 6);
    randombytes_buf(uuid.begin() + 6, 2);
    randombytes_buf(uuid.begin() + 8, 8);

    *(uuid.begin() + 6) &= 0x0f;
    *(uuid.begin() + 6) |= 0x40;

    *(uuid.begin() + 8) &= 0x3f;
    *(uuid.begin() + 8) |= 0x80;

    return uuid;
}

auto Uuid::uuid7() noexcept -> Uuid {
    Uuid uuid;

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

auto Uuid::data() noexcept -> Pointer { return data_.data(); }
auto Uuid::data() const noexcept -> ConstPointer { return data_.data(); }

auto Uuid::size() const noexcept -> SizeType { return data_.size(); }

auto Uuid::version() const noexcept -> UuidVersion {
    const auto &octet9 = data_[6];
    if ((octet9 & 0xf0) == 0x40) {
        return UuidVersion::RandomNumber;
    } else if ((octet9 & 0xf0) == 0x70) {
        return UuidVersion::UnixTime;
    } else {
        return UuidVersion::Unknown;
    }
}

auto Uuid::is_nil() const noexcept -> bool {
    return std::all_of(data_.begin(), data_.end(), [](uint8_t byte) { return byte == 0; });
}

auto Uuid::to_string() const -> std::string {
    return fmt::format(
        "{:02x}{:02x}{:02x}{:02x}-"
        "{:02x}{:02x}-"
        "{:02x}{:02x}-"
        "{:02x}{:02x}-"
        "{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
        data_[0], data_[1], data_[2], data_[3],
        data_[4], data_[5],
        data_[6], data_[7],
        data_[8], data_[9],
        data_[10], data_[11], data_[12], data_[13], data_[14], data_[15]
    );
}

auto Uuid::begin() noexcept -> Iterator { return data_.begin(); }
auto Uuid::begin() const noexcept -> ConstIterator { return data_.begin(); }

auto Uuid::end() noexcept -> Iterator { return data_.end(); }
auto Uuid::end() const noexcept -> ConstIterator { return data_.end(); }

auto Uuid::operator==(const Uuid &rhs) noexcept -> bool {
    return data_ == rhs.data_;
}

auto Uuid::operator<=>(const Uuid &rhs) noexcept -> std::strong_ordering {
    return data_ <=> rhs.data_;
}

}

auto std::hash<muon::Uuid>::operator()(const muon::Uuid &uuid) const -> size_t {
    const uint64_t *ptr = reinterpret_cast<const uint64_t *>(uuid.data());
    size_t hash1 = std::hash<uint64_t>{}(ptr[0]);
    size_t hash2 = std::hash<uint64_t>{}(ptr[1]);
    return hash1 ^ (hash2 << 1);
}

auto fmt::formatter<muon::Uuid>::format(const muon::Uuid &uuid, format_context &ctx) const -> format_context::iterator {
    return formatter<string_view>::format(uuid.to_string(), ctx);
}
