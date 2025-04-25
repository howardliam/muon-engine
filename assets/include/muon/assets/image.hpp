#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>
#include <optional>
#include "muon/assets/file.hpp"

namespace muon::assets {

    enum class ColorFormat {
        Rgb,
        Rgba,
    };

    struct Size {
        uint32_t width;
        uint32_t height;
    };

    struct Image {
        Size size;
        ColorFormat format;
        uint8_t bitDepth;
        std::vector<uint8_t> data;
    };

    std::optional<Image> loadImage(const std::filesystem::path &path);
    std::optional<Image> loadImage(const std::vector<uint8_t> &encodedData, ImageFormat format);

    std::optional<std::vector<uint8_t>> encodeImage(const Image &image, ImageFormat format);

}
