#include "muon/asset/model/gltf.hpp"

int main() {
    using namespace muon;

    asset::parseGltf("./test/assets/models/trex.glb", asset::GltfFileType::Binary);
    asset::parseGltf("./test/assets/models/test.gltf", asset::GltfFileType::Json);
}
