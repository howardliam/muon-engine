#pragma once

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
        std::optional<std::string> name{};
    };

}
