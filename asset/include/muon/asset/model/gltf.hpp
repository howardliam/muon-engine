#pragma once

#include "muon/asset/error.hpp"
#include "muon/asset/model/scene.hpp"
#include <cstdint>
#include <expected>
#include <filesystem>
#include <vector>

namespace muon::asset {

    struct GltfIntermediate {
        std::filesystem::path path;
        std::vector<uint8_t> json{};
        std::vector<std::vector<uint8_t>> bufferData{};
    };

    std::expected<GltfIntermediate, AssetLoadError> intermediateFromGltf(const std::filesystem::path &path);

    std::expected<GltfIntermediate, AssetLoadError> intermediateFromGlb(const std::filesystem::path &path);

    std::expected<Scene, AssetLoadError> parseGltf(const GltfIntermediate &intermediate);

}
