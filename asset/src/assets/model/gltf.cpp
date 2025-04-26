#include "muon/assets/model/gltf.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace muon::assets {

    void parseGltf(const std::vector<uint8_t> &data) {
        const json gltf = json::parse(data);
    }

}
