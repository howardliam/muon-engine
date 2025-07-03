#pragma once

#include "muon/schematic/pipeline/common.hpp"
#include <bitset>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    struct ColorBlendAttachmentInfo {
        bool blendEnable{false};
        std::optional<BlendFactor> srcColorBlendFactor{std::nullopt};
        std::optional<BlendFactor> dstColorBlendFactor{std::nullopt};
        std::optional<BlendOp> colorBlendOp{std::nullopt};
        std::optional<BlendFactor> srcAlphaBlendFactor{std::nullopt};
        std::optional<BlendFactor> dstAlphaBlendFactor{std::nullopt};
        std::optional<BlendOp> alphaBlendOp{std::nullopt};
        std::optional<std::bitset<4>> colorWriteMask{std::nullopt};

        constexpr auto ToVk() const -> VkPipelineColorBlendAttachmentState {
            VkPipelineColorBlendAttachmentState state{};

            state.blendEnable = blendEnable;
            if (blendEnable) {
                state.srcColorBlendFactor = static_cast<VkBlendFactor>(*srcColorBlendFactor);
                state.dstColorBlendFactor = static_cast<VkBlendFactor>(*dstColorBlendFactor);
                state.colorBlendOp = static_cast<VkBlendOp>(*colorBlendOp);
                state.srcAlphaBlendFactor = static_cast<VkBlendFactor>(*srcAlphaBlendFactor);
                state.dstAlphaBlendFactor = static_cast<VkBlendFactor>(*dstAlphaBlendFactor);
                state.alphaBlendOp = static_cast<VkBlendOp>(*colorBlendOp);
                state.colorWriteMask = colorWriteMask->to_ulong();
            }

            return state;
        }
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<ColorBlendAttachmentInfo> {
        static auto to_json(json &j, const auto &info) {
            j["blendEnable"] = info.blendEnable;
            if (info.blendEnable) {
                j["srcColorBlendFactor"] = static_cast<uint32_t>(*info.srcColorBlendFactor);
                j["dstColorBlendFactor"] = static_cast<uint32_t>(*info.dstColorBlendFactor);
                j["colorBlendOp"] = static_cast<uint32_t>(*info.colorBlendOp);
                j["srcAlphaBlendFactor"] = static_cast<uint32_t>(*info.srcAlphaBlendFactor);
                j["dstAlphaBlendFactor"] = static_cast<uint32_t>(*info.dstAlphaBlendFactor);
                j["alphaBlendOp"] = static_cast<uint32_t>(*info.alphaBlendOp);
                j["colorWriteMask"] = info.colorWriteMask->to_ulong();
            }
        }

        static auto from_json(const json &j, auto &info) {
            info.blendEnable = j["blendEnable"].get<bool>();
            if (info.blendEnable) {
                info.srcColorBlendFactor = j["srcColorBlendFactor"].get<BlendFactor>();
                info.dstColorBlendFactor = j["dstColorBlendFactor"].get<BlendFactor>();
                info.colorBlendOp = j["colorBlendOp"].get<BlendOp>();
                info.srcAlphaBlendFactor = j["srcAlphaBlendFactor"].get<BlendFactor>();
                info.dstAlphaBlendFactor = j["dstAlphaBlendFactor"].get<BlendFactor>();
                info.alphaBlendOp = j["alphaBlendOp"].get<BlendOp>();
                info.colorWriteMask = j["colorWriteMask"].get<uint8_t>();
            }
        }
    };

}
