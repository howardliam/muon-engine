#pragma once

#include <array>
#include <vulkan/vulkan_core.h>

namespace muon::graphics {

constexpr std::array<const char *, 1> k_requiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

} // namespace muon::graphics
