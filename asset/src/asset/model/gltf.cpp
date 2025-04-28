#include "muon/asset/model/gltf.hpp"

#include <fstream>
#include <memory>
#include <print>
#include <utility>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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

    std::optional<Scene> parseGltf(const std::vector<uint8_t> &data, const std::filesystem::path &path) {
        const json gltf = json::parse(data);

        std::vector<std::unique_ptr<std::ifstream>> buffers{};
        auto gltfBuffers = gltf["buffers"];
        for (auto gltfBuffer : gltfBuffers) {
            auto bufferPath = path.parent_path().append(std::string(gltfBuffer["uri"]));

            buffers.push_back(std::make_unique<std::ifstream>(bufferPath, std::ios::binary));
        }

        auto gltfBufferViews = gltf["bufferViews"];

        auto gltfAccessors = gltf["accessors"];

        std::vector<std::unique_ptr<Mesh>> meshes{};
        auto gltfMeshes = gltf["meshes"];
        for (auto gltfMesh : gltfMeshes) {
            int32_t vertexSize{0};
            int32_t vertexStride{0};
            int32_t vertexDataSize{0};
            auto gltfAttributes = gltfMesh["primitives"][0]["attributes"];
            for (int32_t accessorIndex : gltfAttributes) {
                auto gltfAccessor = gltfAccessors[accessorIndex];
                int32_t attributeStride = getByteOffset(gltfAccessor["componentType"], gltfAccessor["type"]);
                vertexStride += attributeStride;
                int32_t attributeCount = gltfAccessor["count"];

                vertexDataSize += (vertexStride * attributeCount);
                vertexSize = gltfAccessor["count"];
            }

            std::unique_ptr mesh = std::make_unique<Mesh>();
            mesh->name = gltfMesh["name"];
            mesh->vertexData.resize(vertexDataSize);
            mesh->vertexSize = vertexSize;

            for (int32_t accessorIndex : gltfAttributes) {
                auto gltfAccessor = gltfAccessors[accessorIndex];
                size_t attributeStride = getByteOffset(gltfAccessor["componentType"], gltfAccessor["type"]);
                int32_t bufferViewIndex = gltfAccessor["bufferView"];
                auto gltfBufferView = gltfBufferViews[bufferViewIndex];

                int32_t byteOffset = gltfBufferView["byteOffset"];
                int32_t byteLength = gltfBufferView["byteLength"];
                int32_t bufferIndex = gltfBufferView["buffer"];
                std::ifstream *buffer = buffers[bufferIndex].get();

                int32_t vertex{0};
                for (int32_t i = 0; i < byteLength; i += attributeStride) {
                    int32_t binaryPosition = byteOffset + i;
                    buffer->seekg(binaryPosition, std::ios::beg);

                    uint8_t attributeData[attributeStride];
                    buffer->read(reinterpret_cast<char *>(attributeData), attributeStride);

                    std::memcpy(
                        &mesh->vertexData[(vertex * vertexStride) + (bufferViewIndex * attributeStride)],
                        attributeData,
                        attributeStride
                    );

                    vertex += 1;
                }
            }

            if (gltfMesh["primitives"][0].contains("indices")) {
                int32_t gltfIndex = gltfMesh["primitives"][0]["indices"];
                auto gltfAccessor = gltfAccessors[gltfIndex];
                int32_t attributeStride = getByteOffset(gltfAccessor["componentType"], gltfAccessor["type"]);
                int32_t numberIndices = gltfAccessor["count"];
                mesh->indices.resize(numberIndices);
                int32_t indexComponentType = gltfAccessor["componentType"];

                int32_t bufferViewIndex = gltfAccessor["bufferView"];
                auto gltfBufferView = gltfBufferViews[bufferViewIndex];
                int32_t bufferIndex = gltfBufferView["buffer"];
                std::ifstream *buffer = buffers[bufferIndex].get();

                if (indexComponentType == UNSIGNED_SHORT) {
                    for (int32_t i = 0; i < numberIndices; i++) {
                        uint16_t index{0};
                        buffer->read(reinterpret_cast<char *>(&index), sizeof(uint16_t));
                        mesh->indices[i] = index;
                    }
                } else if (indexComponentType == UNSIGNED_INT) {
                    for (int32_t i = 0; i < numberIndices; i++) {
                        uint32_t index{0};
                        buffer->read(reinterpret_cast<char *>(&index), sizeof(uint32_t));
                        mesh->indices[i] = index;
                    }
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
