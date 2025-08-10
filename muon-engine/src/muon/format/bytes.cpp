#include "muon/format/bytes.hpp"

#include "fmt/format.h"

#include <array>

namespace muon::format {

constexpr std::array<const char *, 4> byte_suffixes = {"B", "kB", "MB", "GB"};

auto bytes(size_t byte_count) -> std::string {
    double size = static_cast<double>(byte_count);
    size_t index = 0;
    while (size >= 1000.0 && index < (byte_suffixes.size() - 1)) {
        size /= 1000.0;
        index += 1;
    }

    return fmt::format("{:.2f} {}", size, byte_suffixes[index]);
}

} // namespace muon::format
