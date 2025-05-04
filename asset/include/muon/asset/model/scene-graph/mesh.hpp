#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace muon::asset::sg {

    using AttributeData = std::variant<
        std::vector<int8_t>,
        std::vector<uint8_t>,
        std::vector<int16_t>,
        std::vector<uint16_t>,
        std::vector<uint32_t>,
        std::vector<float>
    >;

    struct Mesh {
        std::string name{};
        std::unordered_map<std::string, AttributeData> attributes{};
        std::vector<uint32_t> indices{};
    };

}
