#pragma once

#include "vulkan/vulkan.hpp"

#include <cstdint>

namespace muon::graphics {

constexpr uint32_t k_vulkanApiVersion = vk::ApiVersion14;

constexpr uint32_t k_maxFramesInFlight = 2;

constexpr uint64_t k_waitDuration = 30'000'000'000;

}
