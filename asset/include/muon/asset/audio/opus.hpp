#pragma once

#include "muon/asset/audio.hpp"
#include <optional>

namespace muon::asset {

    std::optional<Audio> decodeOpus(const std::vector<uint8_t> &data);

}
