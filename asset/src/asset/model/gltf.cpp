#include "muon/asset/model/gltf.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace muon::asset {

    void testing() {
        const std::filesystem::path path = "./test/assets/models/trex.glb";
        std::println("> loading model: {}", path.string());

        std::ifstream gltfFile{path, std::ios::binary};

        struct Header {
            uint32_t magic;
            uint32_t version;
            uint32_t length;
        };

        Header header;
        gltfFile.read(reinterpret_cast<char *>(&header), sizeof(header));

        std::println("> reading header");
        std::println("\t > magic: {:#x}", header.magic);
        std::println("\t > version: {}", header.version);
        std::println("\t > length: {} bytes", header.length);

        if (header.magic == 0x46'54'6C'67 && header.version == 2) {
            std::println("> validated header, this is glTF 2.0 binary");
        } else {
            return;
        }

        struct Chunk {
            uint32_t length;
            uint32_t type;
        };

        Chunk jsonChunk;
        gltfFile.read(reinterpret_cast<char *>(&jsonChunk), sizeof(jsonChunk));

        std::println("> reading JSON chunk");
        std::println("\t > length: {} bytes", jsonChunk.length);
        std::println("\t > type: {:#x}", jsonChunk.type);

        if (jsonChunk.type == 0x4E'4F'53'4A) {
            std::println("> validated chunk, this is JSON");
        } else {
            return;
        }

        std::println("> reading JSON data");
        std::vector<uint8_t> jsonBytes(jsonChunk.length);
        gltfFile.read(reinterpret_cast<char *>(jsonBytes.data()), jsonBytes.size());

        json gltf = json::parse(jsonBytes);
        std::cout << gltf["asset"] << std::endl;

        size_t filePos = gltfFile.tellg();
        std::println("> at position: {} in file", filePos);
        if (filePos == header.length) {
            std::println("> at end of file, exiting");
            return;
        }

        Chunk binChunk;
        gltfFile.read(reinterpret_cast<char *>(&binChunk), sizeof(binChunk));

        std::println("> reading binary chunk");
        std::println("\t > length: {} bytes", binChunk.length);
        std::println("\t > type: {:#x}", binChunk.type);

        if (binChunk.type == 0x00'4E'49'42) {
            std::println("> validated chunk, this is binary");
        } else {
            return;
        }

        std::println("> reading binary data");
        std::vector<uint8_t> binBytes(binChunk.length);
        gltfFile.read(reinterpret_cast<char *>(binBytes.data()), binBytes.size());

        filePos = gltfFile.tellg();
        std::println("> at position: {} in file", filePos);
        if (filePos == header.length) {
            std::println("> at end of file, exiting");
            return;
        }
    }

}
