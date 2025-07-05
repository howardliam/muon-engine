#pragma once

#include <bitset>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    struct ColorBlendAttachmentInfo {
        bool blendEnable{false};
        std::optional<VkBlendFactor> srcColorBlendFactor{std::nullopt};
        std::optional<VkBlendFactor> dstColorBlendFactor{std::nullopt};
        std::optional<VkBlendOp> colorBlendOp{std::nullopt};
        std::optional<VkBlendFactor> srcAlphaBlendFactor{std::nullopt};
        std::optional<VkBlendFactor> dstAlphaBlendFactor{std::nullopt};
        std::optional<VkBlendOp> alphaBlendOp{std::nullopt};
        std::optional<std::bitset<4>> colorWriteMask{std::nullopt};

        constexpr auto ToVk() const -> VkPipelineColorBlendAttachmentState {
            VkPipelineColorBlendAttachmentState state{};

            state.blendEnable = blendEnable;
            if (blendEnable) {
                state.srcColorBlendFactor = *srcColorBlendFactor;
                state.dstColorBlendFactor = *dstColorBlendFactor;
                state.colorBlendOp = *colorBlendOp;
                state.srcAlphaBlendFactor = *srcAlphaBlendFactor;
                state.dstAlphaBlendFactor = *dstAlphaBlendFactor;
                state.alphaBlendOp = *colorBlendOp;
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
                info.srcColorBlendFactor = j["srcColorBlendFactor"].get<VkBlendFactor>();
                info.dstColorBlendFactor = j["dstColorBlendFactor"].get<VkBlendFactor>();
                info.colorBlendOp = j["colorBlendOp"].get<VkBlendOp>();
                info.srcAlphaBlendFactor = j["srcAlphaBlendFactor"].get<VkBlendFactor>();
                info.dstAlphaBlendFactor = j["dstAlphaBlendFactor"].get<VkBlendFactor>();
                info.alphaBlendOp = j["alphaBlendOp"].get<VkBlendOp>();
                info.colorWriteMask = j["colorWriteMask"].get<uint8_t>();
            }
        }
    };

}
