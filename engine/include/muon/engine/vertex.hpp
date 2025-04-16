#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    struct IVertexLayout {
        virtual std::vector<vk::VertexInputBindingDescription> getBindingDescriptions() const = 0;
        virtual std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions() const = 0;
    };

}
