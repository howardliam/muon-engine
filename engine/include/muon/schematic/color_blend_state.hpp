#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <magic_enum/magic_enum.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <vector>

namespace muon::schematic {

    enum class BlendFactor {
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
        SrcAlphaSATURATE,
        Src1Color,
        OneMinusSrc1Color,
        Src1Alpha,
        OneMinusSrc1Alpha,
    };

    enum class BlendOp {
        Add = 0,
        Subtract = 1,
        ReverseSubtract = 2,
        Min = 3,
        Max = 4,
        ZeroExt = 1000148000,
        SrcExt = 1000148001,
        DstExt = 1000148002,
        SrcOverExt = 1000148003,
        DstOverExt = 1000148004,
        SrcInExt = 1000148005,
        DstInExt = 1000148006,
        SrcOutExt = 1000148007,
        DstOutExt = 1000148008,
        SrcAtopExt = 1000148009,
        DstAtopExt = 1000148010,
        XorExt = 1000148011,
        MultiplyExt = 1000148012,
        ScreenExt = 1000148013,
        OverlayExt = 1000148014,
        DarkenExt = 1000148015,
        LightenExt = 1000148016,
        ColorDodgeExt = 1000148017,
        ColorBurnExt = 1000148018,
        HardLightExt = 1000148019,
        SoftLightExt = 1000148020,
        DifferenceExt = 1000148021,
        ExclusionExt = 1000148022,
        InvertExt = 1000148023,
        InvertRgbExt = 1000148024,
        LinearDodgeExt = 1000148025,
        LinearBurnExt = 1000148026,
        VividLightExt = 1000148027,
        LinearLightExt = 1000148028,
        PinLightExt = 1000148029,
        HardmixExt = 1000148030,
        HslHueExt = 1000148031,
        HslSaturationExt = 1000148032,
        HslColorExt = 1000148033,
        HslLuminosityExt = 1000148034,
        PlusExt = 1000148035,
        PlusClampedExt = 1000148036,
        PlusClampedAlphaExt = 1000148037,
        PlusDarkerExt = 1000148038,
        MinusExt = 1000148039,
        MinusClampedExt = 1000148040,
        ContrastExt = 1000148041,
        InvertOvgExt = 1000148042,
        RedExt = 1000148043,
        GreenExt = 1000148044,
        BlueExt = 1000148045,
    };

    struct ColorBlendAttachment {
        bool blendEnable{false};
        std::optional<BlendFactor> srcColorBlendFactor;
        std::optional<BlendFactor> dstColorBlendFactor;
        std::optional<BlendOp> colorBlendOp;
        std::optional<BlendFactor> srcAlphaBlendFactor;
        std::optional<BlendFactor> dstAlphaBlendFactor;
        std::optional<BlendOp> alphaBlendOp;
        std::bitset<4> colorWriteMask;
    };

    enum class LogicOp {
        Clear,
        And,
        AndReverse,
        Copy,
        AndInverted,
        NoOp,
        Xor,
        Or,
        Nor,
        Equivalent,
        Invert,
        OrReverse,
        CopyInverted,
        OrInverted,
        Nand,
        Set,
    };

    struct ColorBlendState {
        bool logicOpEnable{false};
        std::optional<LogicOp> logicOp{std::nullopt};
        std::vector<ColorBlendAttachment> attachments{};
        std::array<float, 4> blendConstants{0.0};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<ColorBlendAttachment> {
        static auto to_json(json &j, const ColorBlendAttachment &attachment) {
            j["blendEnable"] = attachment.blendEnable;
            if (attachment.blendEnable) {
                j["srcColorBlendFactor"] = *magic_enum::enum_index(*attachment.srcColorBlendFactor);
                j["dstColorBlendFactor"] = *magic_enum::enum_index(*attachment.dstColorBlendFactor);
                j["colorBlendOp"] = *magic_enum::enum_index(*attachment.colorBlendOp);
                j["srcAlphaBlendFactor"] = *magic_enum::enum_index(*attachment.srcAlphaBlendFactor);
                j["dstAlphaBlendFactor"] = *magic_enum::enum_index(*attachment.dstAlphaBlendFactor);
                j["alphaBlendOp"] = *magic_enum::enum_index(*attachment.alphaBlendOp);
                j["colorWriteMask"] = attachment.colorWriteMask.to_ulong();
            }
        }

        static auto from_json(const json &j, ColorBlendAttachment &attachment) {
            attachment.blendEnable = j["blendEnable"].get<bool>();
            if (attachment.blendEnable) {
                attachment.srcColorBlendFactor = j["srcColorBlendFactor"].get<BlendFactor>();
                attachment.dstColorBlendFactor = j["dstColorBlendFactor"].get<BlendFactor>();
                attachment.colorBlendOp = j["colorBlendOp"].get<BlendOp>();
                attachment.srcAlphaBlendFactor = j["srcAlphaBlendFactor"].get<BlendFactor>();
                attachment.dstAlphaBlendFactor = j["dstAlphaBlendFactor"].get<BlendFactor>();
                attachment.alphaBlendOp = j["alphaBlendOp"].get<BlendOp>();
                attachment.colorWriteMask = j["colorWriteMask"].get<uint8_t>();
            }
        }
    };

    template<>
    struct adl_serializer<ColorBlendState> {
        static auto to_json(json &j, const ColorBlendState &state) {
            j["logicOpEnable"] = state.logicOpEnable;
            if (state.logicOpEnable && state.logicOp.has_value()) {
                j["logicOp"] = *magic_enum::enum_index(*state.logicOp);
            }

            for (const auto &attachment : state.attachments) {
                j["attachments"].push_back(attachment);
            }

            j["blendConstants"] = state.blendConstants;
        }

        static auto from_json(const json &j, ColorBlendState &state) {
            state.logicOpEnable = j["logicOpEnable"].get<bool>();
            if (state.logicOpEnable && j.contains("logicOp")) {
                state.logicOp = j["logicOp"].get<LogicOp>();
            }

            if (j.contains("attachments") && j["attachments"].is_array()) {
                state.attachments.reserve(j["attachments"].size());

                for (const auto &attachment : j["attachments"]) {
                    state.attachments.push_back(attachment.get<ColorBlendAttachment>());
                }
            }

            state.blendConstants = j["blendConstants"].get<std::array<float, 4>>();
        }
    };

}
