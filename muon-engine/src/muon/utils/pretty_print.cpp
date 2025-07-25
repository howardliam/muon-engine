#include "muon/utils/pretty_print.hpp"

#include <array>
#include <fmt/format.h>

namespace muon::pp {

constexpr std::array<const char *, 4> k_byteSuffixes = {"B", "kB", "MB", "GB"};

auto printBytes(uint64_t byteCount) -> std::string {
    double size = static_cast<double>(byteCount);
    size_t index = 0;

    while (size >= 1000.0 && index < (k_byteSuffixes.size() - 1)) {
        size /= 1000.0;
        index += 1;
    }

    return fmt::format("{:.2f} {}", size, k_byteSuffixes[index]);
}

} // namespace muon::pp
