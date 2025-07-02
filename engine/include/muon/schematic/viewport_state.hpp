#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>

namespace muon::schematic {

    struct ViewportState {
        uint32_t viewportCount{1};
        uint32_t scissorCount{1};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<ViewportState> {
        static auto to_json(json &j, const ViewportState &state) {
            j["viewportCount"] = state.viewportCount;
            j["scissorCount"] = state.scissorCount;
        }

        static auto from_json(const json &j, ViewportState &state) {
            state.viewportCount = j["viewportCount"].get<uint32_t>();
            state.scissorCount = j["scissorCount"].get<uint32_t>();
        }
    };

}
