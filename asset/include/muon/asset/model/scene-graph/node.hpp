#pragma once

#include <memory>
#include <string>
#include <vector>

namespace muon::asset::sg {

    struct Node {
        std::string name;
        std::weak_ptr<Node> parent;
        std::vector<std::shared_ptr<Node>> children{};
    };

}
