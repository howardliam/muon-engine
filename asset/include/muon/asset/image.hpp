#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>
#include <optional>
#include "muon/asset/file.hpp"

namespace muon::asset {

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

    /**
     * @brief   loads and decodes the image at the path provided.
     *
     * @param   path    path of the image.
     *
     * @return  optional image info if reading and decoding was successful.
     */
    [[nodiscard]] std::optional<Image> loadImage(const std::filesystem::path &path);

    /**
     * @brief   decodes the image with the encoded data and format provided.
     *
     * @param   encodedData encoded data of the image.
     * @param   format      the file format of the image; PNG, JPEG, etc.
     *
     * @return  optional image info if decoding was successful.
     */
    [[nodiscard]] std::optional<Image> loadImage(const std::vector<uint8_t> &encodedData, ImageFormat format);

    /**
     * @brief   encodes the raw image data to the format provided.
     *
     * @param   image   image data.
     * @param   format  the file format of the image; PNG, JPEG, etc.
     *
     * @return  optional vector of bytes if encoding was successful.
     */
    [[nodiscard]] std::optional<std::vector<uint8_t>> encodeImage(const Image &image, ImageFormat format);

}
