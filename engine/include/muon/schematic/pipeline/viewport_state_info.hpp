#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    struct ViewportStateInfo {
        uint32_t viewportCount{1};
        uint32_t scissorCount{1};

        constexpr auto ToVk() const -> VkPipelineViewportStateCreateInfo {
            VkPipelineViewportStateCreateInfo info{};

            info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            info.viewportCount = viewportCount;
            info.pViewports = nullptr;
            info.scissorCount = scissorCount;
            info.pScissors = nullptr;

            return info;
        }
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<ViewportStateInfo> {
        static auto to_json(json &j, const auto &info) {
            j["viewportCount"] = info.viewportCount;
            j["scissorCount"] = info.scissorCount;
        }

        static auto from_json(const json &j, auto &info) {
            info.viewportCount = j["viewportCount"].get<uint32_t>();
            info.scissorCount = j["scissorCount"].get<uint32_t>();
        }
    };

}
