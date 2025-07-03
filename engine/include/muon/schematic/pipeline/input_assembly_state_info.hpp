#pragma once

#include "muon/schematic/pipeline/common.hpp"
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    struct InputAssemblyStateInfo {
        PrimitiveTopology topology;
        bool primitiveRestartEnable{false};

        constexpr auto ToVk() const -> VkPipelineInputAssemblyStateCreateInfo {
            VkPipelineInputAssemblyStateCreateInfo info{};

            info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            info.topology = static_cast<VkPrimitiveTopology>(topology);
            info.primitiveRestartEnable = primitiveRestartEnable;

            return info;
        }
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<InputAssemblyStateInfo> {
        static auto to_json(json &j, const auto &info) {
            j["topology"] = static_cast<uint32_t>(info.topology);
            j["primitiveRestartEnable"] = info.primitiveRestartEnable;
        }

        static auto from_json(const json &j, auto &info) {
            info.topology = j["topology"].get<PrimitiveTopology>();
            info.primitiveRestartEnable = j["primitiveRestartEnable"].get<bool>();
        }
    };

}
