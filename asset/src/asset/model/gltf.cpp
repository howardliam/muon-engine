#include "muon/asset/model/gltf.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace muon::asset {

    Gltf parseGltfBinary(const std::filesystem::path &path) {
        std::ifstream gltfFile{path, std::ios::binary};

        struct Header {
            uint32_t magic;
            uint32_t version;
            uint32_t length;
        };

        Header header;
        gltfFile.read(reinterpret_cast<char *>(&header), sizeof(header));
        if (header.magic != 0x46'54'6C'67 || header.version != 2) {
            return {};
        }

        struct Chunk {
            uint32_t length;
            uint32_t type;
        };

        Chunk jsonChunk;
        gltfFile.read(reinterpret_cast<char *>(&jsonChunk), sizeof(jsonChunk));
        if (jsonChunk.type != 0x4E'4F'53'4A) {
            return {};
        }

        std::vector<uint8_t> jsonBytes(jsonChunk.length);
        gltfFile.read(reinterpret_cast<char *>(jsonBytes.data()), jsonBytes.size());
        if (gltfFile.tellg() == header.length) {
            return {};
        }

        Chunk binChunk;
        gltfFile.read(reinterpret_cast<char *>(&binChunk), sizeof(binChunk));
        if (binChunk.type != 0x00'4E'49'42) {
            return {};
        }

        std::vector<uint8_t> binBytes(binChunk.length);
        gltfFile.read(reinterpret_cast<char *>(binBytes.data()), binBytes.size());

        return {
            .path = path,
            .jsonData = jsonBytes,
            .binaryData = binBytes,
        };
    }

    Gltf parseGltfJson(const std::filesystem::path &path) {
        std::ifstream gltfFile{path, std::ios::binary | std::ios::ate};
        std::vector<uint8_t> jsonBytes(gltfFile.tellg());
        gltfFile.seekg(0, std::ios::beg);
        gltfFile.read(reinterpret_cast<char *>(jsonBytes.data()), jsonBytes.size());

        json gltf = json::parse(jsonBytes);

        std::filesystem::path binPath = path.parent_path().append(std::string(gltf["buffers"][0]["uri"]));
        std::ifstream binFile{binPath, std::ios::binary | std::ios::ate};
        std::vector<uint8_t> binBytes(binFile.tellg());
        binFile.seekg(0, std::ios::beg);
        binFile.read(reinterpret_cast<char *>(binBytes.data()), binBytes.size());

        return {
            .path = path,
            .jsonData = jsonBytes,
            .binaryData = binBytes,
        };
    }

    void parseGltf(const std::filesystem::path &path, const GltfFileType gltfType) {
        Gltf intermediate = [](const std::filesystem::path &path, const GltfFileType gltfType) {
            switch (gltfType) {
            case GltfFileType::Json:
                return parseGltfJson(path);
            case GltfFileType::Binary:
                return parseGltfBinary(path);
            }
        }(path, gltfType);

        const json gltf = json::parse(intermediate.jsonData);

        uint32_t sceneIndex = gltf["scene"];
        std::cout << gltf["scenes"][sceneIndex] << std::endl;
    }
}
