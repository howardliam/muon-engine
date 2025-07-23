#pragma once

#include "muon/schematic/pipeline/color_blend_attachment_info.hpp"

#include <array>
#include <cstdint>
#include <nlohmann/adl_serializer.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

struct ColorBlendStateInfo {
    bool logicOpEnable{false};
    std::optional<VkLogicOp> logicOp{std::nullopt};
    std::vector<ColorBlendAttachmentInfo> attachments{};
    std::array<float, 4> blendConstants{0.0};

    constexpr auto ToVk() const
        -> std::tuple<VkPipelineColorBlendStateCreateInfo, std::vector<VkPipelineColorBlendAttachmentState>> {
        VkPipelineColorBlendStateCreateInfo info{};
        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
        colorBlendAttachments.reserve(attachments.size());
        for (const auto &attachment : attachments) {
            colorBlendAttachments.push_back(attachment.ToVk());
        }

        info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        info.logicOpEnable = logicOpEnable;
        if (logicOpEnable) {
            info.logicOp = *logicOp;
        }
        info.attachmentCount = colorBlendAttachments.size();
        info.pAttachments = colorBlendAttachments.data();
        info.blendConstants[0] = blendConstants[0];
        info.blendConstants[1] = blendConstants[1];
        info.blendConstants[2] = blendConstants[2];
        info.blendConstants[3] = blendConstants[3];

        return {info, colorBlendAttachments};
    }
};

} // namespace muon::schematic

namespace nlohmann {

using namespace muon::schematic;

template <>
struct adl_serializer<ColorBlendStateInfo> {
    static auto to_json(json &j, const auto &info) {
        j["logicOpEnable"] = info.logicOpEnable;
        if (info.logicOpEnable && info.logicOp.has_value()) {
            j["logicOp"] = static_cast<uint32_t>(*info.logicOp);
        }

        for (const auto &attachment : info.attachments) {
            j["attachments"].push_back(attachment);
        }

        j["blendConstants"] = info.blendConstants;
    }

    static auto from_json(const json &j, auto &info) {
        info.logicOpEnable = j["logicOpEnable"].get<bool>();
        if (info.logicOpEnable && j.contains("logicOp")) {
            info.logicOp = j["logicOp"].get<VkLogicOp>();
        }

        if (j.contains("attachments") && j["attachments"].is_array()) {
            info.attachments.reserve(j["attachments"].size());

            for (const auto &attachment : j["attachments"]) {
                info.attachments.push_back(attachment.get<ColorBlendAttachmentInfo>());
            }
        }

        info.blendConstants = j["blendConstants"].get<std::array<float, 4>>();
    }
};

} // namespace nlohmann
