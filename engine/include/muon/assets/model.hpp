#pragma once

#include <assimp/mesh.h>
#include <filesystem>
#include <optional>

namespace muon::assets {

    std::optional<aiMesh *> loadModel(const std::filesystem::path &modelPath);

}
