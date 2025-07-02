#pragma once

#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>

namespace muon::schematic {

    enum class RasterizationSamples {
        Count1 = 0x1,
        Count2 = 0x2,
        Count4 = 0x4,
        Count8 = 0x8,
        Count16 = 0x10,
        Count32 = 0x20,
        Count64 = 0x40,
    };

    struct MultisampleState {
        RasterizationSamples rasterizationSamples;
        bool sampleShadingEnable{false};
        std::optional<float> minSampleShading{std::nullopt};
        bool alphaToCoverageEnable{false};
        bool alphaToOneEnable{false};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<MultisampleState> {
        static auto to_json(json &j, const MultisampleState &state) {
            j["rasterizationSamples"] = *magic_enum::enum_index(state.rasterizationSamples);
            j["sampleShadingEnable"] = state.sampleShadingEnable;
            if (state.sampleShadingEnable && state.minSampleShading.has_value()) {
                j["minSampleShading"] = *state.minSampleShading;
            }
            j["alphaToCoverageEnable"] = state.alphaToCoverageEnable;
            j["alphaToOneEnable"] = state.alphaToOneEnable;
        }

        static auto from_json(const json &j, MultisampleState &state) {
            state.rasterizationSamples = j["rasterizationSamples"].get<RasterizationSamples>();
            state.sampleShadingEnable = j["sampleShadingEnable"].get<bool>();
            if (state.sampleShadingEnable && j.contains("minSampleShading")) {
                state.minSampleShading = j["minSampleShading"].get<float>();
            }
            state.alphaToCoverageEnable = j["alphaToCoverageEnable"].get<bool>();
            state.alphaToOneEnable = j["alphaToOneEnable"].get<bool>();
        }
    };

}
