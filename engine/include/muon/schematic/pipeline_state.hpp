#pragma once

#include "muon/schematic/color_blend_state.hpp"
#include "muon/schematic/input_assembly_state.hpp"
#include "muon/schematic/multisample_state.hpp"
#include "muon/schematic/rasterization_state.hpp"
#include "muon/schematic/viewport_state.hpp"
#include <cstdint>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    struct StencilOpState {
        VkStencilOp failOp;
        VkStencilOp passOp;
        VkStencilOp depthFailOp;
        VkCompareOp compareOp;
        uint32_t compareMask{0};
        uint32_t writeMask{0};
        uint32_t reference{0};
    };

    struct DepthStencilState {
        bool depthTestEnable{true};
        bool depthWriteEnable{true};
        VkCompareOp depthCompareOp;
        bool depthBoundsTestEnable{false};
        float minDepthBounds{0.0};
        float maxDepthBounds{1.0};
        bool stencilTestEnable{false};
        std::optional<StencilOpState> front{std::nullopt};
        std::optional<StencilOpState> back{std::nullopt};
    };

    struct DynamicState {
        std::vector<VkDynamicState> states{};
    };

    struct PipelineState {
        std::optional<InputAssemblyState> inputAssembly{}; // only used by graphics pipelines
        ViewportState viewport{};
        RasterizationState rasterization{};
        MultisampleState multisample{};
        ColorBlendState colorBlend{};
        DepthStencilState depthStencil{};
        DynamicState dynamic{};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<PipelineState> {
        static auto to_json(json &j, const PipelineState &state) {
            if (state.inputAssembly.has_value()) {
                j["inputAssembly"] = *state.inputAssembly;
            }
            j["viewport"] = state.viewport;
            j["rasterization"] = state.rasterization;
            j["multisample"] = state.multisample;
            j["colorBlend"] = state.colorBlend;
        }

        static auto from_json(const json &j, PipelineState &state) {
            if (j.contains("inputAssembly")) {
                state.inputAssembly = j["inputAssembly"].get<InputAssemblyState>();
            }
            state.viewport = j["viewport"].get<ViewportState>();
            state.rasterization = j["rasterization"].get<RasterizationState>();
            state.multisample = j["multisample"].get<MultisampleState>();
            state.colorBlend = j["colorBlend"].get<ColorBlendState>();
        }
    };

}
