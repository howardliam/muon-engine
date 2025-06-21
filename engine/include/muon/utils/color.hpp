#pragma once

#include <array>
#include <cstdint>

namespace muon::color {

    template<typename T>
    T RgbFromHex(uint32_t hex);

    template<>
    std::array<float, 3> RgbFromHex<std::array<float, 3>>(uint32_t hex);

    template<typename T>
    T RgbFromHex(uint8_t r, uint8_t g, uint8_t b);

    template<>
    std::array<float, 3> RgbFromHex<std::array<float, 3>>(uint8_t r, uint8_t g, uint8_t b);

    template<typename T>
    T RgbaFromHex(uint32_t hex);

    template<>
    std::array<float, 4> RgbaFromHex<std::array<float, 4>>(uint32_t hex);

    template<typename T>
    T RgbaFromHex(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    template<>
    std::array<float, 4> RgbaFromHex<std::array<float, 4>>(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

}
