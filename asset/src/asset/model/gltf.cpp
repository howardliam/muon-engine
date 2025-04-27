#include "muon/asset/model/gltf.hpp"

#include <nlohmann/json.hpp>
#include <print>

using json = nlohmann::json;

namespace muon::asset {

    std::optional<Scene> parseGltf(const std::vector<uint8_t> &data) {
        const json gltf = json::parse(data);

        Scene scene{};

        int32_t sceneIndex = gltf["scene"];

        scene.name = gltf["scenes"][sceneIndex]["name"];
        std::vector<int32_t> nodeIndices = gltf["scenes"][sceneIndex]["nodes"];
        for (auto index : nodeIndices) {
            std::println("{}", index);
        }

        return {};
    }

}
