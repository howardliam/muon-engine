#pragma once

#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>

namespace muon::schematic {

    enum class PolygonMode {
        Fill,
        Line,
        Point,
    };

    enum class CullMode {
        None,
        Front,
        Back,
    };

    enum class FrontFace {
        CounterClockwise,
        Clockwise,
    };

    struct RasterizationState {
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
    struct adl_serializer<RasterizationState> {
        static auto to_json(json &j, const RasterizationState &state) {
            j["polygonMode"] = *magic_enum::enum_index(state.polygonMode);
            if (state.polygonMode == PolygonMode::Line && state.lineWidth.has_value()) {
                j["lineWidth"] = *state.lineWidth;
            }
            j["cullMode"] = *magic_enum::enum_index(state.cullMode);
            j["frontFace"] = *magic_enum::enum_index(state.frontFace);

            j["rasterizerDiscardEnable"] = state.rasterizerDiscardEnable;
            j["depthClampEnable"] = state.depthClampEnable;
            j["depthBiasEnable"] = state.depthBiasEnable;
            j["depthBiasConstantFactor"] = state.depthBiasConstantFactor;
            j["depthBiasClamp"] = state.depthBiasClamp;
            j["depthBiasSlopeFactor"] = state.depthBiasSlopeFactor;
        }

        static auto from_json(const json &j, RasterizationState &state) {
            state.polygonMode = j["polygonMode"].get<PolygonMode>();
            if (state.polygonMode == PolygonMode::Line && j.contains("lineWidth")) {
                state.lineWidth = j["lineWidth"].get<float>();
            }
            state.cullMode = j["cullMode"].get<CullMode>();
            state.frontFace = j["frontFace"].get<FrontFace>();

            state.rasterizerDiscardEnable = j["rasterizerDiscardEnable"].get<bool>();
            state.depthClampEnable = j["depthClampEnable"].get<bool>();
            state.depthBiasEnable = j["depthBiasEnable"].get<bool>();
            state.depthBiasConstantFactor = j["depthBiasConstantFactor"].get<float>();
            state.depthBiasClamp = j["depthBiasClamp"].get<float>();
            state.depthBiasSlopeFactor = j["depthBiasSlopeFactor"].get<float>();
        }
    };

}
