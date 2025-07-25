#pragma once

#include <array>

namespace muon::graphics {

constexpr std::array<const char *, 4> k_deviceRequiredExtensions = {
    "VK_KHR_swapchain", "VK_EXT_mesh_shader", "VK_EXT_memory_budget", "VK_EXT_extended_dynamic_state3"
};

constexpr std::array<const char *, 4> k_instanceRequiredExtensions = {
    "VK_EXT_swapchain_colorspace",
    "VK_KHR_get_surface_capabilities2",
    "VK_EXT_display_surface_counter",
    "VK_KHR_display",
};

} // namespace muon::graphics
