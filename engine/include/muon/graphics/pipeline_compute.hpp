#pragma once

#include "muon/graphics/pipeline_layout.hpp"
#include "muon/utils/nocopy.hpp"
#include "muon/utils/nomove.hpp"
#include <filesystem>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

    struct PipelineComputeSpecification {
        VkDevice device{nullptr};
        std::shared_ptr<PipelineLayout> layout{nullptr};
        std::filesystem::path path{};
    };

    class PipelineCompute : NoCopy, NoMove {
    public:
        PipelineCompute(const PipelineComputeSpecification &spec);
        ~PipelineCompute();

    private:
        VkPipeline m_pipeline{nullptr};
    };

}
