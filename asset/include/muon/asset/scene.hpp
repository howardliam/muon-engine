#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace muon::asset {

    struct Mesh {
        std::optional<std::string> name{};

        std::vector<uint8_t> vertexData{};
        uint32_t vertexSize{0};
        std::vector<uint32_t> indices{};

    };

    struct Node {
        std::optional<std::string> name{};

        std::shared_ptr<Node> parentNode{nullptr};
        std::vector<std::shared_ptr<Node>> childNodes{};

        std::unique_ptr<Mesh> mesh{nullptr};
    };

    struct Scene {
        std::optional<std::string> name{};

        std::vector<std::shared_ptr<Node>> rootNodes{};
    };

}
