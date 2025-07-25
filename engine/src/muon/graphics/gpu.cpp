#include "muon/graphics/gpu.hpp"

#include "vulkan/vulkan.hpp"

#include <unordered_set>

namespace muon::graphics {

Gpu::Gpu(const Spec &spec) { determineSuitability(*spec.physicalDevice, *spec.surface); }

bool Gpu::isSuitable() const { return m_coreSuitabilities == 0b1110; }

uint64_t Gpu::getMemorySize() const { return m_memorySize; }

const std::unordered_set<std::string> &Gpu::getSupportedExtensions() const { return m_supportedExtensions; }

void Gpu::determineSuitability(const vk::raii::PhysicalDevice &physicalDevice, const vk::raii::SurfaceKHR &surface) {
    auto deviceProperties = physicalDevice.getProperties();

    if (deviceProperties.apiVersion >= vk::ApiVersion13) {
        m_coreSuitabilities.set(3);
    }

    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        m_coreSuitabilities.set(2);
    }

    if (deviceProperties.limits.maxPushConstantsSize >= 128) {
        m_coreSuitabilities.set(1);
    }

    auto memoryProperties = physicalDevice.getMemoryProperties();

    for (const auto &heap : memoryProperties.memoryHeaps) {
        if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
            m_memorySize += heap.size;
        }
    }
}

} // namespace muon::graphics
