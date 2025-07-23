#pragma once

#include <array>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

constexpr std::array<const char *, 6> k_requiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,         VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    VK_EXT_MESH_SHADER_EXTENSION_NAME,       VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
};

constexpr std::array<const char *, 2> k_optionalDeviceExtensions = {
    VK_AMD_DISPLAY_NATIVE_HDR_EXTENSION_NAME,
    VK_EXT_HDR_METADATA_EXTENSION_NAME,
};

} // namespace muon::graphics
