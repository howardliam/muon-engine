#pragma once

#include "muon/schematic/color_blend_state_info.hpp"
#include "muon/schematic/depth_stencil_state_info.hpp"
#include "muon/schematic/dynamic_state_info.hpp"
#include "muon/schematic/input_assembly_state_info.hpp"
#include "muon/schematic/multisample_state_info.hpp"
#include "muon/schematic/rasterization_state_info.hpp"
#include "muon/schematic/viewport_state_info.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    struct PipelineStateInfo {
        std::optional<InputAssemblyStateInfo> inputAssembly{}; // only used by graphics pipelines
        ViewportStateInfo viewport{};
        RasterizationStateInfo rasterization{};
        MultisampleStateInfo multisample{};
        ColorBlendStateInfo colorBlend{};
        DepthStencilStateInfo depthStencil{};
        DynamicStateInfo dynamic{};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<PipelineStateInfo> {
        static auto to_json(json &j, const auto &info) {
            if (info.inputAssembly.has_value()) {
                j["inputAssembly"] = *info.inputAssembly;
            }
            j["viewport"] = info.viewport;
            j["rasterization"] = info.rasterization;
            j["multisample"] = info.multisample;
            j["colorBlend"] = info.colorBlend;
            j["depthStencil"] = info.depthStencil;
            j["dynamic"] = info.dynamic;
        }

        static auto from_json(const json &j, auto &info) {
            if (j.contains("inputAssembly")) {
                info.inputAssembly = j["inputAssembly"].get<InputAssemblyStateInfo>();
            }
            info.viewport = j["viewport"].get<ViewportStateInfo>();
            info.rasterization = j["rasterization"].get<RasterizationStateInfo>();
            info.multisample = j["multisample"].get<MultisampleStateInfo>();
            info.colorBlend = j["colorBlend"].get<ColorBlendStateInfo>();
            info.depthStencil = j["depthStencil"].get<DepthStencilStateInfo>();
            info.dynamic = j["dynamic"].get<DynamicStateInfo>();
        }
    };

}
