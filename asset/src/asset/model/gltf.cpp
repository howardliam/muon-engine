#include "muon/asset/model/gltf.hpp"

#include <nlohmann/json.hpp>
#include <print>

using json = nlohmann::json;

#define BYTE 5120
#define UNSIGNED_BYTE 5121

#define SHORT 5122
#define UNSIGNED_SHORT 5123

#define UNSIGNED_INT 5125
#define FLOAT 5126

namespace muon::asset {

    size_t getByteOffset(const int32_t componentType, const std::string &type) {
        auto getNumBytes = [](const int32_t componentType) -> size_t {
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

        auto getByteFactor = [](const std::string &type) -> size_t {
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

        size_t bytes = getNumBytes(componentType);
        size_t byteFactor = getByteFactor(type);

        return bytes * byteFactor;
    }

    std::optional<Scene> parseGltf(const std::vector<uint8_t> &data) {
        const json gltf = json::parse(data);

        std::vector<std::unique_ptr<Mesh>> meshes{};

        auto gltfMeshes = gltf["meshes"];
        for (auto gltfMesh : gltfMeshes) {
            std::unique_ptr mesh = std::make_unique<Mesh>();
            mesh->name = gltfMesh["name"];

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
