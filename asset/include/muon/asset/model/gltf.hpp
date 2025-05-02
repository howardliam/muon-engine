#pragma once

#include "muon/asset/error.hpp"
#include "muon/asset/model/scene/scene.hpp"
#include <cstdint>
#include <expected>
#include <filesystem>
#include <vector>

namespace muon::asset {

    struct GltfIntermediate {
        std::filesystem::path path;
        std::vector<uint8_t> json{};
        std::vector<std::vector<uint8_t>> bufferData{};

        // only used for glTF JSON format.
        std::vector<std::vector<uint8_t>> imageData{};
    };

    struct GlbHeader {
        uint32_t magic;
        uint32_t version;
        uint32_t length;
    };

    struct GlbChunkHeader {
        uint32_t length;
        uint32_t type;
    };

    std::expected<GltfIntermediate, AssetLoadError> intermediateFromGltf(const std::filesystem::path &path);

    std::expected<GltfIntermediate, AssetLoadError> intermediateFromGlb(const std::filesystem::path &path);

    std::expected<Scene, AssetLoadError> parseGltf(const GltfIntermediate &intermediate);

}
