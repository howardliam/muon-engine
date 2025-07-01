#include "muon/schematic/pipeline.hpp"
#include "muon/schematic/shader.hpp"

#include <catch2/catch_test_macros.hpp>

namespace muon::schematic {

    using json = nlohmann::json;
    using namespace nlohmann::literals;

    const json pipelineJson = R"({
      "type": 2,
      "shaders": {
        "0": {
          "entryPoint": "main",
          "path": "foo"
        },
        "1": {
          "entryPoint": "main",
          "path": "foo",
          "workGroupSize": [32, 32, 1]
        },
        "6": {
          "entryPoint": "main",
          "path": "foo"
        }
      }
    })"_json;

    TEST_CASE("pipeline serialization", "[schematic]") {
        Pipeline pipeline{};
        pipeline.type = PipelineType::Meshlet;

        pipeline.shaders[ShaderStage::Task].entryPoint = "main";
        pipeline.shaders[ShaderStage::Task].path = "foo";
        pipeline.shaders[ShaderStage::Task].workGroupSize = std::nullopt;

        pipeline.shaders[ShaderStage::Mesh].entryPoint = "main";
        pipeline.shaders[ShaderStage::Mesh].path = "foo";
        pipeline.shaders[ShaderStage::Mesh].workGroupSize = {32, 32, 1};

        pipeline.shaders[ShaderStage::Fragment].entryPoint = "main";
        pipeline.shaders[ShaderStage::Fragment].path = "foo";
        pipeline.shaders[ShaderStage::Fragment].workGroupSize = std::nullopt;

        const json j = pipeline;
        REQUIRE(j == pipelineJson);
    }

    TEST_CASE("pipeline deserialization", "[schematic]") {
        const auto pipeline = pipelineJson.get<Pipeline>();
        REQUIRE(pipeline.type == PipelineType::Meshlet);
        REQUIRE(pipeline.shaders.size() == 3);
        REQUIRE(pipeline.IsValid());
    }

}
