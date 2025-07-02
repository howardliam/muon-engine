#pragma once

#include "muon/schematic/color_blend_attachment_info.hpp"
#include "muon/schematic/common.hpp"
#include <array>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <optional>
#include <vector>

namespace muon::schematic {

    struct ColorBlendStateInfo {
        bool logicOpEnable{false};
        std::optional<LogicOp> logicOp{std::nullopt};
        std::vector<ColorBlendAttachmentInfo> attachments{};
        std::array<float, 4> blendConstants{0.0};
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
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
                info.logicOp = j["logicOp"].get<LogicOp>();
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

}
