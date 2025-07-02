#pragma once

#include "muon/schematic/pipeline/common.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <vector>

namespace muon::schematic {

    struct DynamicStateInfo {
        std::vector<DynamicState> states{};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<DynamicStateInfo> {
        static auto to_json(json &j, const auto &info) {
            for (const auto &state : info.states) {
                j["states"].push_back(static_cast<uint32_t>(state));
            }
        }

        static auto from_json(const json &j, auto &info) {
            if (!j["states"].is_array()) { return; }

            info.states.reserve(j["states"].size());
            for (const auto &state : j["states"]) {
                info.states.push_back(j["states"].get<DynamicState>());
            }
        }
    };

}
