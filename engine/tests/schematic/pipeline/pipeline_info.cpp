#include "muon/schematic/pipeline/pipeline_info.hpp"

#include <catch2/catch_test_macros.hpp>
#include <vulkan/vulkan_core.h>

namespace muon::schematic {

using json = nlohmann::json;
using namespace nlohmann::literals;

TEST_CASE("Compute pipeline info is serialized", "[schematic][pipeline]") {
    const auto jsonPipelineInfo = R"({
            "shaders": {
                "VK_SHADER_STAGE_COMPUTE_BIT": {
                    "entryPoint": "main",
                    "path": "foo",
                    "workGroupSize": [3, 3, 1]
                }
            },
            "type": "Compute"
        })"_json;

    schematic::PipelineInfo info{};
    info.type = schematic::PipelineType::Compute;

    info.shaders[VK_SHADER_STAGE_COMPUTE_BIT].entryPoint = "main";
    info.shaders[VK_SHADER_STAGE_COMPUTE_BIT].path = "foo";
    info.shaders[VK_SHADER_STAGE_COMPUTE_BIT].workGroupSize = {3, 3, 1};

    const json j = info;

    REQUIRE(j == jsonPipelineInfo);
}

} // namespace muon::schematic
