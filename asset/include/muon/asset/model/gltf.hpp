#pragma once

#include "muon/asset/scene.hpp"
#include <optional>
#include <vector>
#include <cstdint>

namespace muon::asset {

    std::optional<Scene> parseGltf(const std::vector<uint8_t> &data);

}
