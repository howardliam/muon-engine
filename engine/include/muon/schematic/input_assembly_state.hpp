#pragma once

#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>

namespace muon::schematic {

    enum class PrimitiveTopology {
        PointList,
        LineList,
        LineStrip,
        TriangleList,
        TriangleStrip,
        TriangleFan,
        LineListWithAdjacency,
        LineStripWithAdjacency,
        TriangleListWithAdjacency,
        TriangleStripWithAdjacency,
        PatchList,
    };

    struct InputAssemblyState {
        PrimitiveTopology topology;
        bool primitiveRestartEnable{false};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<InputAssemblyState> {
        static auto to_json(json &j, const InputAssemblyState &state) {
            j["topology"] = *magic_enum::enum_index(state.topology);
            j["primitiveRestartEnable"] = state.primitiveRestartEnable;
        }

        static auto from_json(const json &j, InputAssemblyState &state) {
            state.topology = j["topology"].get<PrimitiveTopology>();
            state.primitiveRestartEnable = j["primitiveRestartEnable"].get<bool>();
        }
    };

}
