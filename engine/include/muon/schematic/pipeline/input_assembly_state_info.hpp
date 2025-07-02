#pragma once

#include "muon/schematic/pipeline/common.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>

namespace muon::schematic {

    struct InputAssemblyStateInfo {
        PrimitiveTopology topology;
        bool primitiveRestartEnable{false};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<InputAssemblyStateInfo> {
        static auto to_json(json &j, const auto &info) {
            j["topology"] = static_cast<uint32_t>(info.topology);
            j["primitiveRestartEnable"] = info.primitiveRestartEnable;
        }

        static auto from_json(const json &j, auto &info) {
            info.topology = j["topology"].get<PrimitiveTopology>();
            info.primitiveRestartEnable = j["primitiveRestartEnable"].get<bool>();
        }
    };

}
