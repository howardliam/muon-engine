#pragma once

#include <array>
#include <vulkan/vulkan_core.h>

namespace muon::gfx::constants {

    constexpr std::array<const char *, 5> k_requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_EXT_MESH_SHADER_EXTENSION_NAME,
    };

    constexpr std::array<const char *, 2> k_optionalDeviceExtenions = {
        VK_AMD_DISPLAY_NATIVE_HDR_EXTENSION_NAME,
        VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
    };

}
