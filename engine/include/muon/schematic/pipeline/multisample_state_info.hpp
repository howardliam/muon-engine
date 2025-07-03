#pragma once

#include "muon/schematic/pipeline/common.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    struct MultisampleStateInfo {
        RasterizationSamples rasterizationSamples;
        bool sampleShadingEnable{false};
        std::optional<float> minSampleShading{std::nullopt};
        bool alphaToCoverageEnable{false};
        bool alphaToOneEnable{false};

        constexpr auto ToVk() const -> VkPipelineMultisampleStateCreateInfo {
            VkPipelineMultisampleStateCreateInfo info{};

            info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            info.rasterizationSamples = static_cast<VkSampleCountFlagBits>(rasterizationSamples);
            info.sampleShadingEnable = sampleShadingEnable;
            if (sampleShadingEnable) {
                info.minSampleShading = *minSampleShading;
            }
            info.alphaToCoverageEnable = alphaToCoverageEnable;
            info.alphaToOneEnable = alphaToOneEnable;
            info.pSampleMask = nullptr;

            return info;
        }
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
