#pragma once

#include "muon/asset/error.hpp"
#include "muon/asset/model/scene.hpp"
#include <expected>
#include <filesystem>

namespace muon::asset {

    std::expected<Scene, AssetLoadError> loadGltf(const std::filesystem::path &path);

}
