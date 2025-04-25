#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>
#include <optional>

namespace muon::assets {

    enum class ImageFormat {
        Png,
        Jpeg,
    };

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

    struct ImageData {
        uint32_t width;
        uint32_t height;
        uint8_t bitDepth;
        std::vector<char> data;
    };

    std::optional<Image> loadImage(const std::filesystem::path &path);
    std::optional<Image> loadImage(const std::vector<uint8_t> &encodedData, ImageFormat format);

    ImageData loadImagePng(std::vector<char> imageData);

    std::vector<uint8_t> encodeImagePng(const ImageData &imageData);

}
