#include "muon/asset/model/gltf.hpp"

#include <expected>
#include <fstream>
#include <iostream>
#include <memory>
#include <print>
#include <utility>
#include "muon/asset/error.hpp"
#include "muon/asset/model/scene/material.hpp"
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

    std::expected<json, AssetLoadError> parseJson(const std::vector<uint8_t> &json) {
        try {
            return json::parse(json);
        } catch (const std::exception &) {
            return std::unexpected(AssetLoadError::ParseError);
        }
    }

    std::expected<GltfIntermediate, AssetLoadError> intermediateFromGltf(const std::filesystem::path &path) {
        GltfIntermediate intermediate{};
        intermediate.path = path;

        std::ifstream glb{path, std::ios::binary};
        glb.seekg(0, std::ios::end);
        size_t jsonSize = glb.tellg();
        glb.seekg(0, std::ios::beg);

        intermediate.json.resize(jsonSize);
        glb.read(reinterpret_cast<char *>(intermediate.json.data()), jsonSize);

        std::expected jsonResult = parseJson(intermediate.json);
        if (!jsonResult) {
            return std::unexpected(jsonResult.error());
        }
        const json gltf = jsonResult.value();

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

    std::expected<GltfIntermediate, AssetLoadError> intermediateFromGlb(const std::filesystem::path &path) {
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

    std::vector<std::shared_ptr<Material>> parseMaterials(const json &gltf) {
        auto gltfMaterials = gltf["materials"];
        assert(gltfMaterials.is_array() && "must be a valid materials array");

        size_t materialCount = gltfMaterials.size();
        std::vector<std::shared_ptr<Material>> materials(materialCount);

        size_t materialIndex{0};
        for (auto &gltfMaterial : gltfMaterials) {
            std::shared_ptr material = std::make_unique<Material>();

            if (gltfMaterial.contains("name")) {
                material->name = gltfMaterial["name"];
            }

            if (gltfMaterial.contains("pbrMetallicRoughness")) {
                material->pbrMetallicRoughness.emplace();

                auto gltfPbrMetallicRoughness = gltfMaterial["pbrMetallicRoughness"];

                if (gltfPbrMetallicRoughness.contains("baseColorFactor")) {
                    material->pbrMetallicRoughness->baseColorFactor = gltfPbrMetallicRoughness["baseColorFactor"];
                }

                if (gltfPbrMetallicRoughness.contains("baseColorTexture")) {
                    material->pbrMetallicRoughness->baseColorTexture.emplace();

                    auto gltfBaseColorTexture = gltfPbrMetallicRoughness["baseColorTexture"];

                    material->pbrMetallicRoughness->baseColorTexture->index = gltfBaseColorTexture["index"];

                    if (gltfBaseColorTexture.contains("texCoord")) {
                        material->pbrMetallicRoughness->baseColorTexture->texCoord = gltfBaseColorTexture["texCoord"];
                    }
                }

                if (gltfPbrMetallicRoughness.contains("metallicFactor")) {
                    material->pbrMetallicRoughness->metallicFactor = gltfPbrMetallicRoughness["metallicFactor"];
                }

                if (gltfPbrMetallicRoughness.contains("roughnessFactor")) {
                    material->pbrMetallicRoughness->roughnessFactor = gltfPbrMetallicRoughness["roughnessFactor"];
                }

                if (gltfPbrMetallicRoughness.contains("metallicRoughnessTexture")) {
                    material->pbrMetallicRoughness->metallicRoughnessTexture.emplace();

                    auto gltfMetallicRoughnessTexture = gltfPbrMetallicRoughness["metallicRoughnessTexture"];

                    material->pbrMetallicRoughness->metallicRoughnessTexture->index = gltfMetallicRoughnessTexture["index"];

                    if (gltfMetallicRoughnessTexture.contains("texCoord")) {
                        material->pbrMetallicRoughness->metallicRoughnessTexture->texCoord = gltfMetallicRoughnessTexture["texCoord"];
                    }
                }
            }

            if (gltfMaterial.contains("normalTexture")) {
                material->normalTexture.emplace();

                auto gltfNormalTexture = gltfMaterial["normalTexture"];

                material->normalTexture->index = gltfNormalTexture["index"];

                if (gltfNormalTexture.contains("texCoord")) {
                    material->normalTexture->texCoord = gltfNormalTexture["texCoord"];
                }

                if (gltfNormalTexture.contains("scale")) {
                    material->normalTexture->scale = gltfNormalTexture["scale"];
                }
            }

            if (gltfMaterial.contains("occlusionTexture")) {
                material->occlusionTexture.emplace();

                auto gltfOcclusionTexture = gltfMaterial["occlusionTexture"];

                material->occlusionTexture->index = gltfOcclusionTexture["index"];

                if (gltfOcclusionTexture.contains("texCoord")) {
                    material->occlusionTexture->texCoord = gltfOcclusionTexture["texCoord"];
                }

                if (gltfOcclusionTexture.contains("strength")) {
                    material->occlusionTexture->strength = gltfOcclusionTexture["strength"];
                }
            }

            if (gltfMaterial.contains("emissiveTexture")) {
                material->emissiveTexture.emplace();

                auto gltfEmissiveTexture = gltfMaterial["emissiveTexture"];

                material->emissiveTexture->index = gltfEmissiveTexture["index"];

                if (gltfEmissiveTexture.contains("texCoord")) {
                    material->emissiveTexture->texCoord = gltfEmissiveTexture["texCoord"];
                }
            }

            if (gltfMaterial.contains("emissiveFactor")) {
                material->emissiveFactor = gltfMaterial["emissiveFactor"];
            }

            if (gltfMaterial.contains("alphaMode")) {
                std::string alphaMode = gltfMaterial["alphaMode"];
                if (alphaMode == "OPAQUE") {
                    material->alphaMode = AlphaMode::Opaque;
                } else if (alphaMode == "MASK") {
                    material->alphaMode = AlphaMode::Mask;
                } else if (alphaMode == "BLEND") {
                    material->alphaMode = AlphaMode::Blend;
                }
            }

            if (gltfMaterial.contains("alphaCutoff")) {
                material->alphaCutoff = gltfMaterial["alphaCutoff"];
            }

            if (gltfMaterial.contains("doubleSided")) {
                material->doubleSided = gltfMaterial["doubleSided"];
            }

            materials[materialIndex] = std::move(material);
            materialIndex += 1;
        }

        return materials;
    };

    std::vector<std::unique_ptr<Mesh>> parseMeshes(
        const json &gltf,
        const std::vector<std::shared_ptr<Material>> &materials,
        const GltfIntermediate &intermediate
    ) {
        auto gltfMeshes = gltf["meshes"];
        assert(gltfMeshes.is_array() && "must be a valid meshes array");

        size_t meshCount = gltfMeshes.size();
        std::vector<std::unique_ptr<Mesh>> meshes(meshCount);

        auto gltfAccessors = gltf["accessors"];
        auto gltfBufferViews = gltf["bufferViews"];

        size_t meshIndex{0};
        for (auto &gltfMesh : gltfMeshes) {
            auto gltfPrimitives = gltfMesh["primitives"][0];

            std::unique_ptr mesh = std::make_unique<Mesh>();

            if (gltfMesh.contains("name")) {
                mesh->name = gltfMesh["name"];
            }

            uint32_t vertexDataSize{0};

            auto gltfAttributes = gltfPrimitives["attributes"];
            for (uint32_t accessorIndex : gltfAttributes) {
                auto gltfAccessor = gltfAccessors[accessorIndex];

                uint32_t attributeStride = getByteOffset(gltfAccessor["componentType"], gltfAccessor["type"]);
                uint32_t attributeCount = gltfAccessor["count"];

                vertexDataSize += attributeStride * attributeCount;
                mesh->vertexSize += attributeStride;
                mesh->vertexCount = gltfAccessor["count"];
            }

            mesh->vertexData.resize(vertexDataSize);

            for (int32_t accessorIndex : gltfAttributes) {
                auto gltfAccessor = gltfAccessors[accessorIndex];

                uint32_t attributeStride = getByteOffset(gltfAccessor["componentType"], gltfAccessor["type"]);

                uint32_t bufferViewIndex = gltfAccessor["bufferView"];
                auto gltfBufferView = gltfBufferViews[bufferViewIndex];

                uint32_t byteOffset = gltfBufferView["byteOffset"];
                uint32_t byteLength = gltfBufferView["byteLength"];
                uint32_t bufferIndex = gltfBufferView["buffer"];

                const auto &bufferData = intermediate.bufferData[bufferIndex];

                for (uint32_t vertex = 0; vertex < mesh->vertexCount; vertex++) {
                    uint32_t readPos = byteOffset + vertex * attributeStride;

                    uint32_t vertexOffset = vertex * mesh->vertexSize;
                    uint32_t attributeOffset = bufferViewIndex * attributeStride;

                    std::memcpy(
                        &mesh->vertexData[vertexOffset + attributeOffset],
                        &bufferData[readPos],
                        attributeStride
                    );
                }
            }

            if (gltfPrimitives.contains("indices")) {
                uint32_t gltfIndicesIndex = gltfPrimitives["indices"];
                auto gltfAccessor = gltfAccessors[gltfIndicesIndex];

                uint32_t attributeStride = getByteOffset(gltfAccessor["componentType"], gltfAccessor["type"]);
                uint32_t indexCount = gltfAccessor["count"];
                mesh->indices.resize(indexCount);

                uint32_t bufferViewIndex = gltfAccessor["bufferView"];
                auto gltfBufferView = gltfBufferViews[bufferViewIndex];
                uint32_t byteOffset = gltfBufferView["byteOffset"];
                uint32_t bufferIndex = gltfBufferView["buffer"];

                const auto &bufferData = intermediate.bufferData[bufferIndex];

                for (uint32_t index = 0; index < indexCount; index++) {
                    uint32_t readPos = byteOffset + index * attributeStride;

                    std::memcpy(&mesh->indices[index], &bufferData[readPos], attributeStride);
                }
            }

            if (gltfPrimitives.contains("material")) {
                uint32_t gltfMaterialIndex = gltfPrimitives["indices"];
                auto gltfAccessor = gltfAccessors[gltfMaterialIndex];

                mesh->material = materials[gltfMaterialIndex];
            }

            meshes[meshIndex] = std::move(mesh);
            meshIndex += 1;
        }

        return meshes;
    }

    std::vector<std::shared_ptr<Node>> parseNodes(const json &gltf, std::vector<std::unique_ptr<Mesh>> &meshes) {
        auto gltfNodes = gltf["nodes"];
        assert(gltfNodes.is_array() && "must be a valid nodes array");

        uint32_t nodeCount = gltfNodes.size();
        std::vector<std::shared_ptr<Node>> nodes(nodeCount);

        uint32_t nodeIndex{0};
        for (auto gltfNode : gltfNodes) {
            std::shared_ptr node = std::make_shared<Node>();
            node->name = gltfNode["name"];
            node->mesh = std::move(meshes[gltfNode["mesh"]]);

            nodes[nodeIndex] = std::move(node);
            nodeIndex += 1;
        }

        return nodes;
    }

    std::expected<Scene, AssetLoadError> parseGltf(const GltfIntermediate &intermediate) {
        std::expected jsonResult = parseJson(intermediate.json);
        if (!jsonResult) {
            return std::unexpected(jsonResult.error());
        }
        const json gltf = jsonResult.value();

        auto gltfBufferViews = gltf["bufferViews"];
        auto gltfAccessors = gltf["accessors"];

        std::vector materials = parseMaterials(gltf);
        std::vector meshes = parseMeshes(gltf, materials, intermediate);
        std::vector nodes = parseNodes(gltf, meshes);

        Scene scene{};
        int32_t sceneIndex = gltf["scene"];
        auto gltfScene = gltf["scenes"][sceneIndex];
        scene.name = gltfScene["name"];
        std::vector<int32_t> nodeIndices = gltfScene["nodes"];
        for (auto nodeIndex : nodeIndices) {
            scene.nodes.push_back(std::move(nodes[nodeIndex]));
        }

        return scene;
    }
}
