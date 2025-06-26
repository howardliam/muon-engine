#pragma once

#include <array>
#include <vulkan/vulkan_core.h>

namespace muon::gfx::constants {

	constexpr std::array<const char*, 2> k_requiredInstanceExtensions = {
		VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME,
		VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
	};

}