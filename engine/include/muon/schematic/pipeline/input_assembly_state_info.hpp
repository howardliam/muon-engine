#pragma once

#include <nlohmann/adl_serializer.hpp>
#include <nlohmann/json.hpp>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

struct InputAssemblyStateInfo {
    VkPrimitiveTopology topology;
    bool primitiveRestartEnable{false};

    constexpr auto ToVk() const -> VkPipelineInputAssemblyStateCreateInfo {
        VkPipelineInputAssemblyStateCreateInfo info{};

        info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        info.topology = topology;
        info.primitiveRestartEnable = primitiveRestartEnable;

        return info;
    }
};

} // namespace muon::schematic

namespace nlohmann {

using namespace muon::schematic;

template <>
struct adl_serializer<InputAssemblyStateInfo> {
    static auto to_json(json &j, const auto &info) {
        j["topology"] = static_cast<uint32_t>(info.topology);
        j["primitiveRestartEnable"] = info.primitiveRestartEnable;
    }

    static auto from_json(const json &j, auto &info) {
        info.topology = j["topology"].get<VkPrimitiveTopology>();
        info.primitiveRestartEnable = j["primitiveRestartEnable"].get<bool>();
    }
};

} // namespace nlohmann
