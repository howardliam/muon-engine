#include "muon/asset/model/gltf.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace muon::asset {

    void parseGltf(const std::vector<uint8_t> &data) {
        const json gltf = json::parse(data);
    }

}
