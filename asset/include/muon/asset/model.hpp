#pragma once

#include "muon/asset/model/scene.hpp"
#include <filesystem>
#include <optional>

namespace muon::asset {

    std::optional<Scene> loadGltf(const std::filesystem::path &path);

}
