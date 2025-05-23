#pragma once

#include <array>
#include <cstdint>

namespace mu::color {

    template<typename T>
    T rgbFromHex(uint32_t hex);

    template<>
    std::array<float, 3> rgbFromHex<std::array<float, 3>>(uint32_t hex);

    template<typename T>
    T rgbFromHex(uint8_t r, uint8_t g, uint8_t b);

    template<>
    std::array<float, 3> rgbFromHex<std::array<float, 3>>(uint8_t r, uint8_t g, uint8_t b);

    template<typename T>
    T rgbaFromHex(uint32_t hex);

    template<>
    std::array<float, 4> rgbaFromHex<std::array<float, 4>>(uint32_t hex);

    template<typename T>
    T rgbaFromHex(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    template<>
    std::array<float, 4> rgbaFromHex<std::array<float, 4>>(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

}
