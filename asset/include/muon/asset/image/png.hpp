#pragma once

#include "muon/asset/image.hpp"

namespace muon::asset {

    /**
     * @brief   decodes the image from PNG format with the encoded data.
     *
     * @param   encodedData encoded data of the image.
     *
     * @return  optional image info if decoding was successful.
     */
    [[nodiscard]] std::optional<Image> decodePng(const std::vector<uint8_t> &encodedData);

    /**
     * @brief   encodes the raw image data to PNG format.
     *
     * @param   image   image data.
     *
     * @return  optional vector of bytes if encoding was successful.
     */
    [[nodiscard]] std::optional<std::vector<uint8_t>> encodePng(const Image &image);

}
