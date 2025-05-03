#pragma once

#include "muon/asset/model/scene-graph/node.hpp"
#include <memory>
#include <string>
#include <vector>

namespace muon::asset::sg {

    struct Scene {
        std::string name;
        std::vector<std::shared_ptr<Node>> nodes{};
    };

}
