#pragma once

#include "muon/asset/model/scene.hpp"
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace muon::asset {

    struct GltfIntermediate {
        std::filesystem::path path;
        std::vector<uint8_t> json{};
        std::vector<std::vector<uint8_t>> bufferData{};
    };

    std::optional<GltfIntermediate> intermediateFromGltf(const std::filesystem::path &path);

    std::optional<GltfIntermediate> intermediateFromGlb(const std::filesystem::path &path);

    std::optional<Scene> parseGltf(const GltfIntermediate &intermediate);

}
