#include "muon/graphics/gpu.hpp"

#include "muon/graphics/constants.hpp"

namespace muon::graphics {

Gpu::Gpu(const vk::raii::PhysicalDevice *physicalDevice) : m_physicalDevice(physicalDevice) { determineSuitability(); }

auto Gpu::isSuitable() const -> bool { return m_coreSuitabilities == 0b1110; }

auto Gpu::getMemorySize() const -> uint64_t { return m_memorySize; }

auto Gpu::getPhysicalDevice() const -> const vk::raii::PhysicalDevice & { return *m_physicalDevice; }

auto Gpu::determineSuitability() -> void {
    auto deviceProperties = m_physicalDevice->getProperties();

    if (deviceProperties.apiVersion >= k_vulkanApiVersion) {
        m_coreSuitabilities.set(3);
    }

    if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        m_coreSuitabilities.set(2);
    }

    if (deviceProperties.limits.maxPushConstantsSize >= 128) {
        m_coreSuitabilities.set(1);
    }

    auto memoryProperties = m_physicalDevice->getMemoryProperties();

    for (const auto &heap : memoryProperties.memoryHeaps) {
        if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
            m_memorySize += heap.size;
        }
    }
}

} // namespace muon::graphics
