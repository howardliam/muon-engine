#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace muon::engine {

    struct VertexLayout {
        std::optional<vk::VertexInputBindingDescription> bindingDescription;
        std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
    };

}
