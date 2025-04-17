#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    struct VertexLayout {
        std::optional<vk::VertexInputBindingDescription> bindingDescription;
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    };

}
