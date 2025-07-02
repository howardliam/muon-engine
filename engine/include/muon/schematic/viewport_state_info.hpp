#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>

namespace muon::schematic {

    struct ViewportStateInfo {
        uint32_t viewportCount{1};
        uint32_t scissorCount{1};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<ViewportStateInfo> {
        static auto to_json(json &j, const auto &info) {
            j["viewportCount"] = info.viewportCount;
            j["scissorCount"] = info.scissorCount;
        }

        static auto from_json(const json &j, auto &info) {
            info.viewportCount = j["viewportCount"].get<uint32_t>();
            info.scissorCount = j["scissorCount"].get<uint32_t>();
        }
    };

}
