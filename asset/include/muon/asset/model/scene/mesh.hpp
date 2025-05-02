#pragma once

#include "muon/asset/model/scene/material.hpp"
#include <memory>
#include <optional>
#include <string>
#include <cstdint>
#include <vector>

namespace muon::asset {

    struct Mesh {
        std::vector<uint8_t> vertexData{};
        uint32_t vertexSize{0};
        uint32_t vertexCount{0};
        std::vector<uint32_t> indices{};
        std::shared_ptr<Material> material{nullptr};
        std::optional<std::string> name;
    };

}
