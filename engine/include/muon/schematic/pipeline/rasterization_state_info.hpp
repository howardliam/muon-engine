#pragma once

#include "muon/schematic/pipeline/common.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>

namespace muon::schematic {

    struct RasterizationStateInfo {
        PolygonMode polygonMode;
        std::optional<float> lineWidth{std::nullopt};
        CullMode cullMode;
        FrontFace frontFace;
        bool rasterizerDiscardEnable{false};
        bool depthClampEnable{false};
        bool depthBiasEnable{false};
        float depthBiasConstantFactor{0.0};
        float depthBiasClamp{0.0};
        float depthBiasSlopeFactor{0.0};
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
            j["depthBiasConstantFactor"] = info.depthBiasConstantFactor;
            j["depthBiasClamp"] = info.depthBiasClamp;
            j["depthBiasSlopeFactor"] = info.depthBiasSlopeFactor;
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
            info.depthBiasConstantFactor = j["depthBiasConstantFactor"].get<float>();
            info.depthBiasClamp = j["depthBiasClamp"].get<float>();
            info.depthBiasSlopeFactor = j["depthBiasSlopeFactor"].get<float>();
        }
    };

}
