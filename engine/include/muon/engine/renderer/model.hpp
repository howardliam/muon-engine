#pragma once

#include "glm/glm.hpp"
#include <memory>

namespace muon {

    class Mesh;

    struct Node {
        std::shared_ptr<Mesh> mesh;
        glm::mat4 transform;
        std::vector<std::unique_ptr<Node>> children;
    };

    class Model {
        Node rootNode;
    };

}
