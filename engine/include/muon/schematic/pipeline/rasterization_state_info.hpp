#pragma once

#include "muon/schematic/pipeline/common.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    struct RasterizationStateInfo {
        PolygonMode polygonMode;
        std::optional<float> lineWidth{std::nullopt};
        CullMode cullMode;
        FrontFace frontFace;
        bool rasterizerDiscardEnable{false};
        bool depthClampEnable{false};
        bool depthBiasEnable{false};
        std::optional<float> depthBiasConstantFactor{std::nullopt};
        std::optional<float> depthBiasClamp{std::nullopt};
        std::optional<float> depthBiasSlopeFactor{std::nullopt};

        constexpr auto ToVk() const -> VkPipelineRasterizationStateCreateInfo {
            VkPipelineRasterizationStateCreateInfo info{};

            info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            info.polygonMode = static_cast<VkPolygonMode>(polygonMode);
            if (polygonMode == schematic::PolygonMode::Line) {
                info.lineWidth = *lineWidth;
            }
            info.cullMode = static_cast<VkCullModeFlagBits>(cullMode);
            info.frontFace = static_cast<VkFrontFace>(frontFace);
            info.rasterizerDiscardEnable = rasterizerDiscardEnable;
            info.depthClampEnable = depthClampEnable;
            info.depthBiasEnable = depthBiasEnable;
            if (depthBiasEnable) {
                info.depthBiasConstantFactor = *depthBiasConstantFactor;
                info.depthBiasClamp = *depthBiasClamp;
                info.depthBiasSlopeFactor = *depthBiasSlopeFactor;
            }

            return info;
        }
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<RasterizationStateInfo> {
        static auto to_json(json &j, const auto &info) {
            j["polygonMode"] = static_cast<uint32_t>(info.polygonMode);
            if (info.polygonMode == PolygonMode::Line && info.lineWidth.has_value()) {
                j["lineWidth"] = *info.lineWidth;
            }
            j["cullMode"] = static_cast<uint32_t>(info.cullMode);
            j["frontFace"] = static_cast<uint32_t>(info.frontFace);

            j["rasterizerDiscardEnable"] = info.rasterizerDiscardEnable;
            j["depthClampEnable"] = info.depthClampEnable;

            j["depthBiasEnable"] = info.depthBiasEnable;
            if (info.depthBiasEnable) {
                j["depthBiasConstantFactor"] = *info.depthBiasConstantFactor;
                j["depthBiasClamp"] = *info.depthBiasClamp;
                j["depthBiasSlopeFactor"] = *info.depthBiasSlopeFactor;
            }
        }

        static auto from_json(const json &j, auto &info) {
            info.polygonMode = j["polygonMode"].get<PolygonMode>();
            if (info.polygonMode == PolygonMode::Line && j.contains("lineWidth")) {
                info.lineWidth = j["lineWidth"].get<float>();
            }
            info.cullMode = j["cullMode"].get<CullMode>();
            info.frontFace = j["frontFace"].get<FrontFace>();

            info.rasterizerDiscardEnable = j["rasterizerDiscardEnable"].get<bool>();
            info.depthClampEnable = j["depthClampEnable"].get<bool>();

            info.depthBiasEnable = j["depthBiasEnable"].get<bool>();
            if (info.depthBiasEnable) {
                info.depthBiasConstantFactor = j["depthBiasConstantFactor"].get<float>();
                info.depthBiasClamp = j["depthBiasClamp"].get<float>();
                info.depthBiasSlopeFactor = j["depthBiasSlopeFactor"].get<float>();
            }
        }
    };

}
