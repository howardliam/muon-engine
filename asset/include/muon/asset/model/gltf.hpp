#pragma once

#include "muon/asset/scene.hpp"
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace muon::asset {

    std::optional<Scene> parseGltf(const std::vector<uint8_t> &data, const std::filesystem::path &path);

}
