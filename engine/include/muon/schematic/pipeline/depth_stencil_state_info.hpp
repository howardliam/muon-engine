#pragma once

#include "muon/schematic/pipeline/stencil_op_state_info.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    struct DepthStencilStateInfo {
        bool depthTestEnable{false};
        std::optional<bool> depthWriteEnable{std::nullopt};
        std::optional<VkCompareOp> depthCompareOp{std::nullopt};
        std::optional<bool> depthBoundsTestEnable{std::nullopt};
        std::optional<float> minDepthBounds{std::nullopt};
        std::optional<float> maxDepthBounds{std::nullopt};

        bool stencilTestEnable{false};
        std::optional<StencilOpStateInfo> front{std::nullopt};
        std::optional<StencilOpStateInfo> back{std::nullopt};

        constexpr auto ToVk() const -> VkPipelineDepthStencilStateCreateInfo {
            VkPipelineDepthStencilStateCreateInfo info{};

            info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            info.depthTestEnable = depthTestEnable;
            if (depthTestEnable) {
                info.depthWriteEnable = *depthWriteEnable;
                info.depthCompareOp = *depthCompareOp;
                info.depthBoundsTestEnable = *depthBoundsTestEnable;
                info.minDepthBounds = *minDepthBounds;
                info.maxDepthBounds = *maxDepthBounds;
            }

            info.stencilTestEnable = stencilTestEnable;
            if (stencilTestEnable) {
                info.front = front->ToVk();
                info.back = back->ToVk();
            }

            return info;
        }
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<DepthStencilStateInfo> {
        static auto to_json(json &j, const auto &info) {
            j["depthTestEnable"] = info.depthTestEnable;
            if (info.depthTestEnable) {
                j["depthWriteEnable"] = *info.depthWriteEnable;
                j["depthCompareOp"] = static_cast<uint32_t>(*info.depthCompareOp);
                j["depthBoundsTestEnable"] = *info.depthBoundsTestEnable;
                j["minDepthBounds"] = *info.minDepthBounds;
                j["maxDepthBounds"] = *info.maxDepthBounds;
            }

            j["stencilTestEnable"] = info.stencilTestEnable;
            if (info.stencilTestEnable) {
                j["front"] = *info.front;
                j["back"] = *info.back;
            }
        }

        static auto from_json(const json &j, auto &info) {
            info.depthTestEnable = j["depthTestEnable"].get<bool>();
            if (info.depthTestEnable) {
                info.depthWriteEnable = j["depthWriteEnable"].get<bool>();
                info.depthCompareOp = j["depthCompareOp"].get<VkCompareOp>();
                info.depthBoundsTestEnable = j["depthBoundsTestEnable"].get<bool>();
                info.minDepthBounds = j["minDepthBounds"].get<float>();
                info.maxDepthBounds = j["maxDepthBounds"].get<float>();
            }

            info.stencilTestEnable = j["stencilTestEnable"].get<bool>();
            if (info.stencilTestEnable) {
                info.front = j["front"].get<StencilOpStateInfo>();
                info.back = j["back"].get<StencilOpStateInfo>();
            }
        }
    };

}
