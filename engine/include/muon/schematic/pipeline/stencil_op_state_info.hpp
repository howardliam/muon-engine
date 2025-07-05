#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <nlohmann/adl_serializer.hpp>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

    struct StencilOpStateInfo {
        VkStencilOp failOp;
        VkStencilOp passOp;
        VkStencilOp depthFailOp;
        VkCompareOp compareOp;
        uint32_t compareMask{0};
        uint32_t writeMask{0};
        uint32_t reference{0};

        constexpr auto ToVk() const -> VkStencilOpState {
            VkStencilOpState state{};

            state.failOp = failOp;
            state.passOp = passOp;
            state.depthFailOp = depthFailOp;
            state.compareOp = compareOp;
            state.compareMask = compareMask;
            state.writeMask = writeMask;
            state.reference = reference;

            return state;
        }
    };

}

namespace nlohmann {

    using namespace muon::schematic;

    template<>
    struct adl_serializer<StencilOpStateInfo> {
        static auto to_json(json &j, const auto &info) {
            j["failOp"] = static_cast<uint32_t>(info.failOp);
            j["passOp"] = static_cast<uint32_t>(info.passOp);
            j["depthFailOp"] = static_cast<uint32_t>(info.depthFailOp);
            j["compareOp"] = static_cast<uint32_t>(info.compareOp);
            j["compareMask"] = info.compareMask;
            j["writeMask"] = info.writeMask;
            j["reference"] = info.reference;
        }

        static auto from_json(const json &j, auto &info) {
            info.failOp = j["failOp"].get<VkStencilOp>();
            info.passOp = j["passOp"].get<VkStencilOp>();
            info.depthFailOp = j["depthFailOp"].get<VkStencilOp>();
            info.compareOp = j["compareOp"].get<VkCompareOp>();
            info.compareMask = j["compareMask"].get<uint32_t>();
            info.writeMask = j["writeMask"].get<uint32_t>();
            info.reference = j["reference"].get<uint32_t>();
        }
    };

}
