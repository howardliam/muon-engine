#pragma once

#include <filesystem>
#include <vector>

namespace muon::asset {

    struct Gltf {
        const std::filesystem::path path;
        const std::vector<uint8_t> jsonData;
        const std::vector<uint8_t> binaryData;
    };

    Gltf parseGltfBinary(const std::filesystem::path &path);
    Gltf parseGltfJson(const std::filesystem::path &path);

    enum class GltfFileType {
        Json,
        Binary,
    };

    void parseGltf(const std::filesystem::path &path, const GltfFileType gltfType);
}
