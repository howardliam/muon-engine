#pragma once

#include "muon/schematic/pipeline/common.hpp"
#include "muon/schematic/pipeline/stencil_op_state_info.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>

namespace muon::schematic {

    struct DepthStencilStateInfo {
        bool depthTestEnable{false};
        std::optional<bool> depthWriteEnable{std::nullopt};
        std::optional<CompareOp> depthCompareOp{std::nullopt};
        std::optional<bool> depthBoundsTestEnable{std::nullopt};
        std::optional<float> minDepthBounds{std::nullopt};
        std::optional<float> maxDepthBounds{std::nullopt};

        bool stencilTestEnable{false};
        std::optional<StencilOpStateInfo> front{std::nullopt};
        std::optional<StencilOpStateInfo> back{std::nullopt};
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
                info.depthCompareOp = j["depthCompareOp"].get<CompareOp>();
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
