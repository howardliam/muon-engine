#pragma once

#include "muon/engine/renderer/mesh.hpp"
#include "glm/glm.hpp"
#include <memory>

namespace mu {

    struct Node {
        std::shared_ptr<Mesh> mesh;
        glm::mat4 transform;
        std::vector<std::unique_ptr<Node>> children;
    };

    class Model {
        Node rootNode;
    };

}
