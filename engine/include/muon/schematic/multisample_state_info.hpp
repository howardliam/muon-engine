#pragma once

#include "muon/schematic/common.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>

namespace muon::schematic {

    struct MultisampleStateInfo {
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
    struct adl_serializer<MultisampleStateInfo> {
        static auto to_json(json &j, const auto &info) {
            j["rasterizationSamples"] = static_cast<uint32_t>(info.rasterizationSamples);
            j["sampleShadingEnable"] = info.sampleShadingEnable;
            if (info.sampleShadingEnable && info.minSampleShading.has_value()) {
                j["minSampleShading"] = *info.minSampleShading;
            }
            j["alphaToCoverageEnable"] = info.alphaToCoverageEnable;
            j["alphaToOneEnable"] = info.alphaToOneEnable;
        }

        static auto from_json(const json &j, auto &info) {
            info.rasterizationSamples = j["rasterizationSamples"].get<RasterizationSamples>();
            info.sampleShadingEnable = j["sampleShadingEnable"].get<bool>();
            if (info.sampleShadingEnable && j.contains("minSampleShading")) {
                info.minSampleShading = j["minSampleShading"].get<float>();
            }
            info.alphaToCoverageEnable = j["alphaToCoverageEnable"].get<bool>();
            info.alphaToOneEnable = j["alphaToOneEnable"].get<bool>();
        }
    };

}
