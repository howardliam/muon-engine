#pragma once

#include "muon/asset/model/scene/node.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace muon::asset {

    struct Scene {
        std::vector<std::shared_ptr<Node>> nodes{};
        std::optional<std::string> name;
    };

}
