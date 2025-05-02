#pragma once

#include "muon/asset/model/scene/camera.hpp"
#include "muon/asset/model/scene/mesh.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace muon::asset {

    struct Node {
        std::unique_ptr<Camera> camera{nullptr};
        std::vector<std::shared_ptr<Node>> children{};
        std::unique_ptr<Mesh> mesh{nullptr};
        // mat4 transform
        // quat rotation
        // vec3 scale
        // vec3 translation
        std::optional<std::string> name;
    };

}
