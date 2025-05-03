#include "muon/asset/model/gltf.hpp"

#include <cstring>
#include <expected>
#include <fstream>
#include <iostream>
#include <memory>
#include <print>
#include <stdexcept>
#include <utility>
#include "muon/asset/error.hpp"
#include "muon/asset/image.hpp"
#include "muon/asset/model/scene/material.hpp"
#include "muon/asset/model/scene/sampler.hpp"
#include "muon/asset/model/scene/image.hpp"
#include <nlohmann/json.hpp>

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

#define NEAREST 9728
#define LINEAR 9729
#define NEAREST_MIPMAP_NEAREST 9984
#define LINEAR_MIPMAP_NEAREST 9985
#define NEAREST_MIPMAP_LINEAR 9986
#define LINEAR_MIPMAP_LINEAR 9987

#define CLAMP_TO_EDGE 33071
#define MIRRORED_REPEAT 33648
#define REPEAT 10497

using json = nlohmann::json;

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

        for (uint32_t i = 0; i < gltfBuffers.size(); i++) {
            auto gltfBuffer = gltfBuffers[i];
            intermediate.bufferData[i].resize(gltfBuffer["byteLength"]);

            auto bufferPath = path.parent_path().append(std::string(gltfBuffer["uri"]));
            std::ifstream buffer{bufferPath, std::ios::binary};

            buffer.read(reinterpret_cast<char *>(intermediate.bufferData[i].data()), gltfBuffer["byteLength"]);
        }

        return intermediate;
    }

    std::expected<GltfIntermediate, AssetLoadError> intermediateFromGlb(const std::filesystem::path &path) {
        GltfIntermediate intermediate{};
        intermediate.path = path;

        std::ifstream glb{path, std::ios::binary};

        GlbHeader header;
        glb.read(reinterpret_cast<char *>(&header), sizeof(GlbHeader));

        if (header.magic != GLB_MAGIC) {
            return std::unexpected(AssetLoadError::InvalidFormat);
        } else if (header.version != GLTF_VERSION) {
            return std::unexpected(AssetLoadError::InvalidFormat);
        }

        GlbChunkHeader jsonHeader;
        glb.read(reinterpret_cast<char *>(&jsonHeader), sizeof(GlbChunkHeader));

        if (jsonHeader.type != JSON_MAGIC) {
            return std::unexpected(AssetLoadError::InvalidFormat);
        }

        intermediate.json.resize(jsonHeader.length);
        glb.read(reinterpret_cast<char *>(intermediate.json.data()), jsonHeader.length);

        GlbChunkHeader binaryHeader;
        glb.read(reinterpret_cast<char *>(&binaryHeader), sizeof(GlbChunkHeader));

        if (binaryHeader.type != BINARY_MAGIC) {
            return std::unexpected(AssetLoadError::InvalidFormat);
        }

        intermediate.bufferData.resize(1);
        intermediate.bufferData[0].resize(binaryHeader.length);
        glb.read(reinterpret_cast<char *>(intermediate.bufferData[0].data()), binaryHeader.length);

        return intermediate;
    }

    std::vector<std::shared_ptr<scene::Sampler>> parseSamplers(const json &gltf) {
        auto gltfSamplers = gltf["samplers"];

        uint32_t samplerCount = gltfSamplers.size();
        std::vector<std::shared_ptr<scene::Sampler>> samplers(samplerCount);

        uint32_t samplerIndex{0};
        for (auto gltfSampler : gltfSamplers) {
            std::shared_ptr sampler = std::make_shared<scene::Sampler>();

            if (gltfSampler.contains("magFilter")) {
                uint32_t magFilter = gltfSampler["magFilter"];
                switch (magFilter) {
                case NEAREST:
                    sampler->magFilter = scene::Filter::Nearest;
                    break;

                case LINEAR:
                    sampler->magFilter = scene::Filter::Linear;
                    break;
                }
            }

            if (gltfSampler.contains("minFilter")) {
                uint32_t minFilter = gltfSampler["minFilter"];
                switch (minFilter) {
                case NEAREST:
                    sampler->minFilter = scene::Filter::Nearest;
                    break;

                case LINEAR:
                    sampler->minFilter = scene::Filter::Linear;
                    break;

                case NEAREST_MIPMAP_NEAREST:
                    sampler->minFilter = scene::Filter::NearestMipmapNearest;
                    break;

                case LINEAR_MIPMAP_NEAREST:
                    sampler->minFilter = scene::Filter::LinearMipmapNearest;
                    break;

                case NEAREST_MIPMAP_LINEAR:
                    sampler->minFilter = scene::Filter::NearestMipmapLinear;
                    break;

                case LINEAR_MIPMAP_LINEAR:
                    sampler->minFilter = scene::Filter::LinearMipmapLinear;
                    break;
                }
            }

            if (gltfSampler.contains("wrapS")) {
                uint32_t wrapS = gltfSampler["wrapS"];
                switch (wrapS) {
                case CLAMP_TO_EDGE:
                    sampler->wrapS = scene::WrappingMode::ClampToEdge;
                    break;

                case MIRRORED_REPEAT:
                    sampler->wrapS = scene::WrappingMode::MirroredRepeat;
                    break;

                case REPEAT:
                    sampler->wrapS = scene::WrappingMode::Repeat;
                    break;
                }
            }

            if (gltfSampler.contains("wrapT")) {
                uint32_t wrapT = gltfSampler["wrapT"];
                switch (wrapT) {
                case CLAMP_TO_EDGE:
                    sampler->wrapT = scene::WrappingMode::ClampToEdge;
                    break;

                case MIRRORED_REPEAT:
                    sampler->wrapT = scene::WrappingMode::MirroredRepeat;
                    break;

                case REPEAT:
                    sampler->wrapT = scene::WrappingMode::Repeat;
                    break;
                }
            }

            if (gltfSampler.contains("name")) {
                sampler->name = gltfSampler["name"];
            }

            samplers[samplerIndex] = std::move(sampler);
            samplerIndex += 1;
        }

        return samplers;
    }

    std::vector<std::shared_ptr<scene::Image>> parseImages(const json &gltf, const GltfIntermediate &intermediate) {
        auto gltfImages = gltf["images"];

        uint32_t imageCount = gltfImages.size();
        std::vector<std::shared_ptr<scene::Image>> images(imageCount);

        uint32_t imageIndex{0};
        for (auto gltfImage : gltfImages) {
            std::shared_ptr image = std::make_shared<scene::Image>();

            if (gltfImage.contains("name")) {
                image->name = gltfImage["name"];
            }

            // .gltf
            if (gltfImage.contains("uri")) {
                auto imagePath = intermediate.path.parent_path().append(std::string(gltfImage["uri"]));

                auto maybeImage = loadImage(imagePath);
                if (!maybeImage) {
                    throw std::runtime_error("failed to load image file");
                }

                image->image = *maybeImage;
            }

            // .glb
            else if (gltfImage.contains("bufferView")) {
                uint32_t bufferViewIndex = gltfImage["bufferView"];
                auto gltfBufferView = gltf["bufferViews"][bufferViewIndex];

                uint32_t byteLength = gltfBufferView["byteLength"];
                uint32_t byteOffset = gltfBufferView["byteOffset"];

                std::vector<uint8_t> imageData(byteLength);

                std::memcpy(
                    imageData.data(),
                    &intermediate.bufferData[0][byteOffset],
                    byteLength
                );

                std::optional<Image> maybeImage;
                std::string mimeType = gltfImage["mimeType"];
                if (mimeType == "image/jpeg") {
                    maybeImage = loadImage(imageData, ImageFormat::Jpeg);
                } else if (mimeType == "image/png") {
                    maybeImage = loadImage(imageData, ImageFormat::Png);
                }
                if (!maybeImage) {
                    throw std::runtime_error("failed to load image file");
                }

                image->image = *maybeImage;
            }

            images[imageIndex] = std::move(image);
            imageIndex += 1;
        }

        return images;
    }

    std::vector<std::shared_ptr<scene::Texture>> parseTextures(
        const json &gltf,
        const std::vector<std::shared_ptr<scene::Sampler>> &samplers,
        const std::vector<std::shared_ptr<scene::Image>> &images
    ) {
        auto gltfTextures = gltf["textures"];

        uint32_t textureCount = gltfTextures.size();
        std::vector<std::shared_ptr<scene::Texture>> textures(textureCount);

        uint32_t textureIndex{0};
        for (auto gltfTexture : gltfTextures) {
            std::shared_ptr texture = std::make_shared<scene::Texture>();

            if (gltfTexture.contains("sampler")) {
                uint32_t samplerIndex = gltfTexture["sampler"];

                texture->sampler = samplers[samplerIndex];
            }

            if (gltfTexture.contains("source")) {
                uint32_t sourceIndex = gltfTexture["source"];

                texture->image = images[sourceIndex];
            }

            if (gltfTexture.contains("name")) {
                texture->name = gltfTexture["name"];
            }

            textures[textureIndex] = std::move(texture);
            textureIndex += 1;
        }

        return textures;
    }

    std::vector<std::shared_ptr<scene::Material>> parseMaterials(
        const json &gltf,
        const std::vector<std::shared_ptr<scene::Texture>> &textures
    ) {
        auto gltfMaterials = gltf["materials"];
        assert(gltfMaterials.is_array() && "must be a valid materials array");

        size_t materialCount = gltfMaterials.size();
        std::vector<std::shared_ptr<scene::Material>> materials(materialCount);

        size_t materialIndex{0};
        for (auto &gltfMaterial : gltfMaterials) {
            std::shared_ptr material = std::make_unique<scene::Material>();

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

                    uint32_t textureIndex = gltfBaseColorTexture["index"];
                    material->pbrMetallicRoughness->baseColorTexture->texture = textures[textureIndex];

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

                    uint32_t textureIndex = gltfMetallicRoughnessTexture["index"];
                    material->pbrMetallicRoughness->metallicRoughnessTexture->texture = textures[textureIndex];

                    if (gltfMetallicRoughnessTexture.contains("texCoord")) {
                        material->pbrMetallicRoughness->metallicRoughnessTexture->texCoord = gltfMetallicRoughnessTexture["texCoord"];
                    }
                }
            }

            if (gltfMaterial.contains("normalTexture")) {
                material->normalTexture.emplace();

                auto gltfNormalTexture = gltfMaterial["normalTexture"];

                uint32_t textureIndex = gltfNormalTexture["index"];
                material->normalTexture->texture = textures[textureIndex];

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

                uint32_t textureIndex = gltfOcclusionTexture["index"];
                material->occlusionTexture->texture = textures[textureIndex];

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

                uint32_t textureIndex = gltfEmissiveTexture["index"];
                material->emissiveTexture->texture = textures[textureIndex];

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
                    material->alphaMode = scene::AlphaMode::Opaque;
                } else if (alphaMode == "MASK") {
                    material->alphaMode = scene::AlphaMode::Mask;
                } else if (alphaMode == "BLEND") {
                    material->alphaMode = scene::AlphaMode::Blend;
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

    std::vector<std::unique_ptr<scene::Mesh>> parseMeshes(
        const json &gltf,
        const std::vector<std::shared_ptr<scene::Material>> &materials,
        const GltfIntermediate &intermediate
    ) {
        auto gltfMeshes = gltf["meshes"];
        assert(gltfMeshes.is_array() && "must be a valid meshes array");

        size_t meshCount = gltfMeshes.size();
        std::vector<std::unique_ptr<scene::Mesh>> meshes(meshCount);

        auto gltfAccessors = gltf["accessors"];
        auto gltfBufferViews = gltf["bufferViews"];

        size_t meshIndex{0};
        for (auto &gltfMesh : gltfMeshes) {
            auto gltfPrimitives = gltfMesh["primitives"][0];

            std::unique_ptr mesh = std::make_unique<scene::Mesh>();

            if (gltfMesh.contains("name")) {
                mesh->name = gltfMesh["name"];
            }

            auto gltfAttributes = gltfPrimitives["attributes"];
            std::vector<uint32_t> attributeStrides(gltfAttributes.size());
            for (uint32_t accessorIndex : gltfAttributes) {
                auto gltfAccessor = gltfAccessors[accessorIndex];

                uint32_t attributeStride = getByteOffset(gltfAccessor["componentType"], gltfAccessor["type"]);
                attributeStrides[accessorIndex] = attributeStride;

                uint32_t attributeCount = gltfAccessor["count"];

                mesh->vertexSize += attributeStride;
                mesh->vertexCount = gltfAccessor["count"];
            }

            std::vector<uint32_t> attributeOffsets(gltfAttributes.size());
            for (uint32_t i = 0; i < attributeOffsets.size(); i++) {
                if (i == 0) {
                    attributeOffsets[i] = 0;
                    continue;
                }
                attributeOffsets[i] = attributeOffsets[i - 1] + attributeStrides[i - 1];
            }

            mesh->vertexData.resize(mesh->vertexSize * mesh->vertexCount);

            for (uint32_t accessorIndex : gltfAttributes) {
                auto gltfAccessor = gltfAccessors[accessorIndex];

                std::println("attribute offset: {}", attributeOffsets[accessorIndex]);

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
                    uint32_t attributeOffset = attributeOffsets[accessorIndex];

                    std::memcpy(
                        &mesh->vertexData[vertexOffset + attributeOffset],
                        &bufferData[readPos],
                        attributeStride
                    );

                    if (accessorIndex == 2) {
                        float test[2];
                        std::memcpy(
                            &test,
                            &bufferData[readPos],
                            attributeStride
                        );

                        std::println("u: {}, v: {}", test[0], test[1]);
                    }
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
                uint32_t gltfMaterialIndex = gltfPrimitives["material"];
                auto gltfAccessor = gltfAccessors[gltfMaterialIndex];

                mesh->material = materials[gltfMaterialIndex];
            }

            meshes[meshIndex] = std::move(mesh);
            meshIndex += 1;
        }

        return meshes;
    }

    std::vector<std::shared_ptr<scene::Node>> parseNodes(
        const json &gltf,
        std::vector<std::unique_ptr<scene::Mesh>> &meshes
    ) {
        auto gltfNodes = gltf["nodes"];
        assert(gltfNodes.is_array() && "must be a valid nodes array");

        uint32_t nodeCount = gltfNodes.size();
        std::vector<std::shared_ptr<scene::Node>> nodes(nodeCount);

        uint32_t nodeIndex{0};
        for (auto gltfNode : gltfNodes) {
            std::shared_ptr node = std::make_shared<scene::Node>();
            node->name = gltfNode["name"];
            node->mesh = std::move(meshes[gltfNode["mesh"]]);

            nodes[nodeIndex] = std::move(node);
            nodeIndex += 1;
        }

        return nodes;
    }

    std::expected<scene::Scene, AssetLoadError> parseGltf(const GltfIntermediate &intermediate) {
        std::expected jsonResult = parseJson(intermediate.json);
        if (!jsonResult) {
            return std::unexpected(jsonResult.error());
        }
        const json gltf = jsonResult.value();

        std::vector samplers = parseSamplers(gltf);
        std::vector images = parseImages(gltf, intermediate);
        std::vector textures = parseTextures(gltf, samplers, images);

        std::vector materials = parseMaterials(gltf, textures);
        std::vector meshes = parseMeshes(gltf, materials, intermediate);
        std::vector nodes = parseNodes(gltf, meshes);

        scene::Scene scene{};
        int32_t sceneIndex = gltf["scene"];
        auto gltfScene = gltf["scenes"][sceneIndex];
        scene.name = gltfScene["name"];
        for (uint32_t nodeIndex : gltfScene["nodes"]) {
            scene.nodes.push_back(std::move(nodes[nodeIndex]));
        }

        return scene;
    }
}
