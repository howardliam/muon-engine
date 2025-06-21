#include "muon/utils/pretty_print.hpp"

#include <array>
#include <format>

namespace muon::pp {

    namespace constants {
        constexpr std::array<const char *, 4> byteSuffixes = {
            "B", "kB", "MB", "GB"
        };
    }

    std::string ParseBytes(uint64_t numBytes) {
        double size = static_cast<double>(numBytes);
        size_t index = 0;

        while (size >= 1000.0 && index < (constants::byteSuffixes.size() - 1)) {
            size /= 1000.0;
            index += 1;
        }

        return std::format("{} {}", size, constants::byteSuffixes[index]);
    }

}
