#pragma once

#include <memory>
#include <string>
#include <vector>

namespace muon::asset {

    struct Mesh {
        std::vector<uint8_t> vertexData{};
        uint32_t vertexSize{0};
        std::vector<uint32_t> indices{};

    };

    struct Node {
        std::string name{};

        std::shared_ptr<Node> parentNode{nullptr};
        std::vector<std::shared_ptr<Node>> childNodes{};

        std::shared_ptr<Mesh> mesh{nullptr};
    };

    struct Scene {
        std::string name{};

        std::vector<std::shared_ptr<Node>> rootNodes{};
    };

}
