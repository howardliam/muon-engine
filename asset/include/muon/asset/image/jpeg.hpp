#pragma once

#include "muon/asset/image.hpp"

namespace muon::asset {

    /**
     * @brief   decodes the image from JPEG format with the encoded data.
     *
     * @param   encodedData encoded data of the image.
     *
     * @return  optional image info if decoding was successful.
     */
    [[nodiscard]] std::optional<Image> decodeJpeg(const std::vector<uint8_t> &encodedData);

}
