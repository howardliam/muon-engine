#pragma once

#include "muon/assets/image.hpp"

namespace muon::assets {

    std::optional<Image> decodePng(const std::vector<uint8_t> &encodedData);

}
