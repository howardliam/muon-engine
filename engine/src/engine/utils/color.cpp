#include "muon/engine/utils/color.hpp"

#include <limits>

namespace muon::color {

    namespace constants {
        constexpr uint8_t ubyteMax = std::numeric_limits<uint8_t>().max();
    }

    template<>
    std::array<float, 3> rgbFromHex<std::array<float, 3>>(uint32_t hex) {
        uint8_t r = (hex >> 0x10) & constants::ubyteMax;
        uint8_t g = (hex >> 0x08) & constants::ubyteMax;
        uint8_t b = (hex >> 0x00) & constants::ubyteMax;

        return rgbFromHex<std::array<float, 3>>(r, g, b);
    }

    template<>
    std::array<float, 3> rgbFromHex<std::array<float, 3>>(uint8_t r, uint8_t g, uint8_t b) {
        return {
            static_cast<float>(r) / constants::ubyteMax,
            static_cast<float>(g) / constants::ubyteMax,
            static_cast<float>(b) / constants::ubyteMax
        };
    }

    template<>
    std::array<float, 4> rgbaFromHex<std::array<float, 4>>(uint32_t hex) {
        uint8_t r = (hex >> 0x18) & constants::ubyteMax;
        uint8_t g = (hex >> 0x10) & constants::ubyteMax;
        uint8_t b = (hex >> 0x08) & constants::ubyteMax;
        uint8_t a = (hex >> 0x00) & constants::ubyteMax;

        return rgbaFromHex<std::array<float, 4>>(r, g, b, a);
    }

    template<>
    std::array<float, 4> rgbaFromHex<std::array<float, 4>>(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        return {
            static_cast<float>(r) / constants::ubyteMax,
            static_cast<float>(g) / constants::ubyteMax,
            static_cast<float>(b) / constants::ubyteMax,
            static_cast<float>(a) / constants::ubyteMax,
        };
    }

}
