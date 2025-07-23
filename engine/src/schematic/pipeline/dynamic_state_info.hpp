#pragma once

#include <nlohmann/adl_serializer.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

struct DynamicStateInfo {
    std::vector<VkDynamicState> states{};

    constexpr auto ToVk() const -> std::tuple<VkPipelineDynamicStateCreateInfo, std::vector<VkDynamicState>> {
        VkPipelineDynamicStateCreateInfo info{};
        std::vector<VkDynamicState> dynamicStateEnables{};
        dynamicStateEnables.reserve(states.size());

        dynamicStateEnables = states;

        info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        info.pDynamicStates = dynamicStateEnables.data();
        info.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

        return {info, dynamicStateEnables};
    }
};

} // namespace muon::schematic

namespace nlohmann {

using namespace muon::schematic;

template <>
struct adl_serializer<DynamicStateInfo> {
    static auto to_json(json &j, const auto &info) {
        for (const auto &state : info.states) {
            j["states"].push_back(static_cast<uint32_t>(state));
        }
    }

    static auto from_json(const json &j, auto &info) {
        if (!j["states"].is_array()) {
            return;
        }

        info.states.reserve(j["states"].size());
        for (const auto &state : j["states"]) {
            info.states.push_back(j["states"].get<VkDynamicState>());
        }
    }
};

} // namespace nlohmann
