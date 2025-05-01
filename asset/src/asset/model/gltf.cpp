#include "muon/asset/model/gltf.hpp"

#include <fstream>
#include <memory>
#include <print>
#include <utility>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define GLTF_VERSION 2
#define GLB_MAGIC 0x46'54'6C'67
#define JSON_MAGIC 0x4E'4F'53'4A
#define BINARY_MAGIC 0x00'4E'49'42

#define BYTE 5120
#define UNSIGNED_BYTE 5121

#define SHORT 5122
#define UNSIGNED_SHORT 5123

#define UNSIGNED_INT 5125
#define FLOAT 5126

namespace muon::asset {

    int32_t getByteOffset(const int32_t componentType, const std::string &type) {
        auto getNumBytes = [](const int32_t componentType) -> int32_t {
            switch (componentType) {
            case BYTE:
            case UNSIGNED_BYTE:
                return 1;

            case SHORT:
            case UNSIGNED_SHORT:
                return 2;

            case UNSIGNED_INT:
            case FLOAT:
                return 4;

            default:
                return 0;
            }
        };

        auto getByteFactor = [](const std::string &type) -> int32_t {
            if (type == "SCALAR") {
                return 1;
            } else if (type == "VEC2") {
                return 2;
            } else if (type == "VEC3") {
                return 3;
            } else if (type == "VEC4") {
                return 4;
            } else if (type == "MAT2") {
                return (2 * 2);
            } else if (type == "MAT3") {
                return (3 * 3);
            } else if (type == "MAT4") {
                return (4 * 4);
            }

            return 0;
        };

        int32_t bytes = getNumBytes(componentType);
        int32_t byteFactor = getByteFactor(type);

        return bytes * byteFactor;
    }

    std::optional<GltfIntermediate> intermediateFromGltf(const std::filesystem::path &path) {
        GltfIntermediate intermediate{};
        intermediate.path = path;

        std::ifstream glb{path, std::ios::binary};
        glb.seekg(0, std::ios::end);
        size_t jsonSize = glb.tellg();
        glb.seekg(0, std::ios::beg);

        intermediate.json.resize(jsonSize);
        glb.read(reinterpret_cast<char *>(intermediate.json.data()), jsonSize);

        const json gltf = json::parse(intermediate.json);

        auto gltfBuffers = gltf["buffers"];
        intermediate.bufferData.resize(gltfBuffers.size());

        for (int32_t i = 0; i < gltfBuffers.size(); i++) {
            auto gltfBuffer = gltfBuffers[i];
            intermediate.bufferData[i].resize(gltfBuffer["byteLength"]);

            auto bufferPath = path.parent_path().append(std::string(gltfBuffer["uri"]));
            std::ifstream buffer{bufferPath, std::ios::binary};

            buffer.read(reinterpret_cast<char *>(intermediate.bufferData[i].data()), gltfBuffer["byteLength"]);
        }

        return intermediate;
    }

    struct GlbHeader {
        uint32_t magic;
        uint32_t version;
        uint32_t length;
    };

    struct GlbChunkHeader {
        uint32_t length;
        uint32_t type;
    };

    std::optional<GltfIntermediate> intermediateFromGlb(const std::filesystem::path &path) {
        GltfIntermediate intermediate{};
        intermediate.path = path;

        std::ifstream glb{path, std::ios::binary};

        GlbHeader header;
        glb.read(reinterpret_cast<char *>(&header), sizeof(GlbHeader));

        if (header.magic != GLB_MAGIC) {
            return {};
        } else if (header.version != GLTF_VERSION) {
            return {};
        }

        GlbChunkHeader jsonHeader;
        glb.read(reinterpret_cast<char *>(&jsonHeader), sizeof(GlbChunkHeader));

        if (jsonHeader.type != JSON_MAGIC) {
            return {};
        }

        intermediate.json.resize(jsonHeader.length);
        glb.read(reinterpret_cast<char *>(intermediate.json.data()), jsonHeader.length);

        GlbChunkHeader binaryHeader;
        glb.read(reinterpret_cast<char *>(&binaryHeader), sizeof(GlbChunkHeader));

        if (binaryHeader.type != BINARY_MAGIC) {
            return {};
        }

        intermediate.bufferData.resize(1);
        intermediate.bufferData[0].resize(binaryHeader.length);
        glb.read(reinterpret_cast<char *>(intermediate.bufferData[0].data()), binaryHeader.length);

        return intermediate;
    }

    std::optional<Scene> parseGltf(const GltfIntermediate &intermediate) {
        const json gltf = json::parse(intermediate.json);

        auto gltfBufferViews = gltf["bufferViews"];

        auto gltfAccessors = gltf["accessors"];

        std::vector<std::unique_ptr<Mesh>> meshes{};
        auto gltfMeshes = gltf["meshes"];
        for (auto gltfMesh : gltfMeshes) {
            int32_t vertexCount{0};
            int32_t vertexSize{0};
            int32_t vertexDataSize{0};
            auto gltfAttributes = gltfMesh["primitives"][0]["attributes"];
            for (int32_t accessorIndex : gltfAttributes) {
                auto gltfAccessor = gltfAccessors[accessorIndex];
                int32_t attributeStride = getByteOffset(gltfAccessor["componentType"], gltfAccessor["type"]);
                vertexSize += attributeStride;
                int32_t attributeCount = gltfAccessor["count"];

                vertexDataSize += attributeStride * attributeCount;
                vertexCount = gltfAccessor["count"];
            }

            std::unique_ptr mesh = std::make_unique<Mesh>();
            mesh->name = gltfMesh["name"];
            mesh->vertexData.resize(vertexDataSize);
            mesh->vertexSize = vertexSize;
            mesh->vertexCount = vertexCount;

            for (int32_t accessorIndex : gltfAttributes) {
                auto gltfAccessor = gltfAccessors[accessorIndex];
                size_t attributeStride = getByteOffset(gltfAccessor["componentType"], gltfAccessor["type"]);
                int32_t bufferViewIndex = gltfAccessor["bufferView"];
                auto gltfBufferView = gltfBufferViews[bufferViewIndex];

                int32_t byteOffset = gltfBufferView["byteOffset"];
                int32_t byteLength = gltfBufferView["byteLength"];
                int32_t bufferIndex = gltfBufferView["buffer"];

                const auto &bufferData = intermediate.bufferData[bufferIndex];

                for (int32_t vertex = 0; vertex < mesh->vertexCount; vertex++) {
                    int32_t readPos = byteOffset + vertex * attributeStride;

                    const uint8_t *attributeData = &bufferData[readPos];

                    size_t vertexOffset = vertex * vertexSize;
                    size_t attributeOffset = bufferViewIndex * attributeStride;
                    std::memcpy(
                        &mesh->vertexData[vertexOffset + attributeOffset],
                        attributeData,
                        attributeStride
                    );
                }
            }

            if (gltfMesh["primitives"][0].contains("indices")) {
                int32_t gltfIndex = gltfMesh["primitives"][0]["indices"];
                auto gltfAccessor = gltfAccessors[gltfIndex];
                int32_t attributeStride = getByteOffset(gltfAccessor["componentType"], gltfAccessor["type"]);
                int32_t indexCount = gltfAccessor["count"];
                mesh->indices.resize(indexCount);

                int32_t bufferViewIndex = gltfAccessor["bufferView"];
                auto gltfBufferView = gltfBufferViews[bufferViewIndex];
                int32_t byteOffset = gltfBufferView["byteOffset"];
                int32_t bufferIndex = gltfBufferView["buffer"];

                const auto &bufferData = intermediate.bufferData[bufferIndex];

                for (int32_t index = 0; index < indexCount; index++) {
                    int32_t readPos = byteOffset + index * attributeStride;
                    const uint8_t *indexData = &bufferData[readPos];

                    std::memcpy(&mesh->indices[index], &bufferData[readPos], attributeStride);
                }
            }

            meshes.push_back(std::move(mesh));
        }

        std::vector<std::shared_ptr<Node>> nodes{};
        auto gltfNodes = gltf["nodes"];
        for (auto gltfNode : gltfNodes) {
            std::shared_ptr node = std::make_shared<Node>();
            node->name = gltfNode["name"];
            node->mesh = std::move(meshes[gltfNode["mesh"]]);

            nodes.push_back(std::move(node));
        }

        Scene scene{};
        int32_t sceneIndex = gltf["scene"];
        auto gltfScene = gltf["scenes"][sceneIndex];
        scene.name = gltfScene["name"];
        std::vector<int32_t> nodeIndices = gltfScene["nodes"];
        for (auto nodeIndex : nodeIndices) {
            scene.rootNodes.push_back(std::move(nodes[nodeIndex]));
        }

        return scene;
    }
}
