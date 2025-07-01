#include "muon/schematic/pipeline.hpp"
#include "muon/schematic/shader.hpp"

#include <catch2/catch_test_macros.hpp>

namespace muon::schematic {

    using json = nlohmann::json;
    using namespace nlohmann::literals;

    const json test = R"({
      "type": 2,
      "shaders": [
        {
          "stage": 6,
          "entryPoint": "main",
          "path": "foo"
        },
        {
          "stage": 7,
          "entryPoint": "main",
          "path": "foo",
          "workGroupSize": [32, 32, 1]
        },
        {
          "stage": 4,
          "entryPoint": "main",
          "path": "foo"
        }
      ]
    })"_json;

    TEST_CASE("pipeline serialization", "[schematic]") {
        Pipeline pipeline{};
        pipeline.type = PipelineType::Meshlet;
        pipeline.shaders.resize(3);

        pipeline.shaders[0].stage = ShaderStage::Task;
        pipeline.shaders[0].entryPoint = "main";
        pipeline.shaders[0].path = "foo";
        pipeline.shaders[0].workGroupSize = std::nullopt;

        pipeline.shaders[1].stage = ShaderStage::Mesh;
        pipeline.shaders[1].entryPoint = "main";
        pipeline.shaders[1].path = "foo";
        pipeline.shaders[1].workGroupSize = {32, 32, 1};

        pipeline.shaders[2].stage = ShaderStage::Fragment;
        pipeline.shaders[2].entryPoint = "main";
        pipeline.shaders[2].path = "foo";
        pipeline.shaders[2].workGroupSize = std::nullopt;

        const json j = pipeline;
        REQUIRE(j == test);
    }

    TEST_CASE("pipeline deserialization", "[schematic]") {
        const auto pipeline = test.get<Pipeline>();
        REQUIRE(pipeline.type == PipelineType::Meshlet);
        REQUIRE(pipeline.shaders.size() == 3);
    }

}
