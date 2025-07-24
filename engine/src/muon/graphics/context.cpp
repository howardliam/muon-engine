#include "muon/graphics/context.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/graphics/gpu.hpp"
#include "muon/graphics/instance_extensions.hpp"
#include "muon/graphics/queue.hpp"
#include "muon/graphics/queue_info.hpp"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "vk_mem_alloc.hpp"
#include "vulkan/vulkan_enums.hpp"
#include "vulkan/vulkan_handles.hpp"
#include "vulkan/vulkan_raii.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <expected>
#include <memory>
#include <set>
#include <vector>

#ifdef MU_DEBUG_ENABLED
static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageType,
    const vk::DebugUtilsMessengerCallbackDataEXT *callbackData, void *userData
) {
    switch (messageSeverity) {
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose: {
            MU_VK_DEBUG(callbackData->pMessage);
            break;
        }

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo: {
            MU_VK_INFO(callbackData->pMessage);
            break;
        }

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning: {
            MU_VK_WARN(callbackData->pMessage);
            break;
        }

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError: {
            MU_VK_ERROR(callbackData->pMessage);
            break;
        }

        default: {
            break;
        }
    }

    return false;
}
#endif

namespace muon::graphics {

Context::Context(const Spec &spec) {
    MU_CORE_ASSERT(spec.window, "a window must be present");

    m_context = vk::raii::Context();

    CreateInstance(*spec.window);
#ifdef MU_DEBUG_ENABLED
    CreateDebugMessenger();
#endif
    CreateSurface(*spec.window);
    SelectPhysicalDevice();
    CreateLogicalDevice();
    CreateAllocator();

    MU_CORE_DEBUG("created device");
}

Context::~Context() {
    m_allocator.destroy();
    m_graphicsQueue.reset();
    m_computeQueue.reset();
    m_transferQueue.reset();

    MU_CORE_DEBUG("destroyed device");
}

auto Context::DeviceWaitIdle() -> std::expected<void, vk::Result> {
    m_device.waitIdle();
    return {};
}

auto Context::GetInstance() -> vk::raii::Instance & { return m_instance; }
auto Context::GetInstance() const -> const vk::raii::Instance & { return m_instance; }

auto Context::GetSurface() -> vk::raii::SurfaceKHR & { return m_surface; }
auto Context::GetSurface() const -> const vk::raii::SurfaceKHR & { return m_surface; }

auto Context::GetPhysicalDevice() -> vk::raii::PhysicalDevice & { return m_physicalDevice; }
auto Context::GetPhysicalDevice() const -> const vk::raii::PhysicalDevice & { return m_physicalDevice; }

auto Context::GetDevice() -> vk::raii::Device & { return m_device; }
auto Context::GetDevice() const -> const vk::raii::Device & { return m_device; }

auto Context::GetGraphicsQueue() const -> Queue & { return *m_graphicsQueue; }
auto Context::GetComputeQueue() const -> Queue & { return *m_computeQueue; }
auto Context::GetTransferQueue() const -> Queue & { return *m_transferQueue; }

auto Context::GetAllocator() const -> vma::Allocator { return m_allocator; }

auto Context::CreateInstance(const Window &window) -> void {
    auto extensions = window.GetRequiredExtensions();
    extensions.insert(extensions.end(), k_requiredInstanceExtensions.begin(), k_requiredInstanceExtensions.end());

#ifdef MU_DEBUG_ENABLED
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "Muon";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Muon";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    vk::InstanceCreateInfo instanceCi;
    instanceCi.pApplicationInfo = &appInfo;
    instanceCi.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCi.ppEnabledExtensionNames = extensions.data();

#ifdef MU_DEBUG_ENABLED
    const char *validationLayer = "VK_LAYER_KHRONOS_validation";

    auto checkValidationLayerSupport = [&validationLayer]() -> bool {
        uint32_t propertyCount = 0;
        vkEnumerateInstanceLayerProperties(&propertyCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(propertyCount);
        vkEnumerateInstanceLayerProperties(&propertyCount, availableLayers.data());

        auto it = std::ranges::find_if(availableLayers, [&](const VkLayerProperties &props) -> bool {
            return std::strcmp(props.layerName, validationLayer) == 0;
        });

        if (it == availableLayers.end()) {
            return false;
        }

        return true;
    };

    if (checkValidationLayerSupport()) {
        instanceCi.enabledLayerCount = 1;
        instanceCi.ppEnabledLayerNames = &validationLayer;
    } else {
        MU_CORE_WARN("the validation layer is not available");
    }
#endif

    auto instanceResult = m_context.createInstance(instanceCi);
    MU_CORE_ASSERT(instanceResult, "failed to create instance");
    m_instance = std::move(*instanceResult);
}

#ifdef MU_DEBUG_ENABLED
auto Context::CreateDebugMessenger() -> void {
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCi;
    debugUtilsMessengerCi.pfnUserCallback = DebugCallback;

    debugUtilsMessengerCi.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    debugUtilsMessengerCi.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    auto debugMessengerResult = m_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCi);
    MU_CORE_ASSERT(debugMessengerResult, "failed to create debug messenger");

    m_debugMessenger = std::move(*debugMessengerResult);
}
#endif

auto Context::CreateSurface(const Window &window) -> void {
    auto surfaceResult = window.CreateSurface(m_instance);
    MU_CORE_ASSERT(surfaceResult, "failed to create window surface");
    m_surface = std::move(*surfaceResult);
    MU_CORE_ASSERT(*m_surface, "surface must not be null");
}

auto Context::SelectPhysicalDevice() -> void {
    auto physicalDevicesResult = m_instance.enumeratePhysicalDevices();
    MU_CORE_ASSERT(physicalDevicesResult, "failed to get available GPUs");
    auto physicalDevices = *physicalDevicesResult;

    Gpu::Spec gpuSpec{};
    gpuSpec.surface = &m_surface;

    using GpuPair = std::pair<Gpu, const vk::raii::PhysicalDevice *>;
    std::vector<GpuPair> gpus{};

    for (const auto &physicalDevice : physicalDevices) {
        gpuSpec.physicalDevice = &physicalDevice;
        Gpu gpu(gpuSpec);

        if (gpu.IsSuitable()) {
            gpus.push_back({gpu, &physicalDevice});
        }
    }

    auto sort = [](const GpuPair &a, const GpuPair &b) -> bool { return a.first.GetMemorySize() > b.first.GetMemorySize(); };
    std::sort(gpus.begin(), gpus.end(), sort);

    if (gpus.size() >= 1) {
        auto &[gpu, physicalDevice] = gpus.front();
        m_physicalDevice = *physicalDevice;
    } else {
        MU_CORE_ASSERT("failed to select a suitable GPU");
    }
}

auto Context::CreateLogicalDevice() -> void {
    const QueueInfo queueInfo{m_physicalDevice, m_surface};
    MU_CORE_ASSERT(queueInfo.GetFamilyInfo().size() >= 1, "there must be at least one queue family");
    MU_CORE_ASSERT(queueInfo.GetTotalQueueCount() >= 3, "there must be at least three queues available");

    const auto queueFamilies = queueInfo.GetFamilyInfo();

    auto graphicsFamily =
        std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) { return info.IsGraphicsCapable(); });
    MU_CORE_ASSERT(graphicsFamily != queueFamilies.end(), "there must be a graphics capable queue family");
    MU_CORE_ASSERT(graphicsFamily->IsPresentCapable(), "the graphics capable queue family must support presentation");

    auto computeFamily =
        std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) { return info.IsComputeDedicated(); });
    if (computeFamily == queueFamilies.end()) {
        computeFamily = std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) {
            return info.IsComputeCapable() && info.queueCount > 1;
        });
    }
    MU_CORE_ASSERT(computeFamily != queueFamilies.end(), "there must be a compute capable queue family");

    auto transferFamily =
        std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) { return info.IsTransferDedicated(); });
    if (transferFamily == queueFamilies.end()) {
        transferFamily = std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) {
            return info.IsTransferCapable() && info.queueCount > 1;
        });
    }
    MU_CORE_ASSERT(transferFamily != queueFamilies.end(), "there must be a transfer capable queue family");

    std::map<uint32_t, uint32_t> queueCounts;
    queueCounts[graphicsFamily->index] += 1;
    queueCounts[computeFamily->index] += 1;
    queueCounts[transferFamily->index] += 1;
    std::set<uint32_t> uniqueQueueFamilies = {graphicsFamily->index, computeFamily->index, transferFamily->index};

    std::vector<float> queuePriorities(uniqueQueueFamilies.size(), 1.0);
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilies.size());
    uint32_t index = 0;
    for (const auto family : uniqueQueueFamilies) {
        auto &createInfo = queueCreateInfos[index];
        createInfo.queueFamilyIndex = family;
        createInfo.queueCount = queueCounts[family];
        createInfo.pQueuePriorities = queuePriorities.data();
        index += 1;
    }

    vk::PhysicalDeviceBufferDeviceAddressFeatures bdaFeatures;
    bdaFeatures.bufferDeviceAddress = true;

    vk::PhysicalDeviceMeshShaderFeaturesEXT msFeatures;
    msFeatures.meshShader = true;
    msFeatures.taskShader = true;
    msFeatures.pNext = &bdaFeatures;

    vk::PhysicalDeviceSynchronization2Features syncFeatures;
    syncFeatures.synchronization2 = true;
    syncFeatures.pNext = &msFeatures;

    vk::PhysicalDeviceDynamicRenderingFeatures drFeatures;
    drFeatures.dynamicRendering = true;
    drFeatures.pNext = &syncFeatures;

    vk::PhysicalDeviceDescriptorIndexingFeatures diFeatures;
    diFeatures.descriptorBindingPartiallyBound = true;
    diFeatures.shaderSampledImageArrayNonUniformIndexing = true;
    diFeatures.runtimeDescriptorArray = true;
    diFeatures.descriptorBindingVariableDescriptorCount = true;
    diFeatures.descriptorBindingSampledImageUpdateAfterBind = true;
    diFeatures.descriptorBindingStorageBufferUpdateAfterBind = true;
    diFeatures.descriptorBindingStorageImageUpdateAfterBind = true;
    diFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind = true;
    diFeatures.descriptorBindingUniformBufferUpdateAfterBind = true;
    diFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = true;
    diFeatures.shaderUniformBufferArrayNonUniformIndexing = true;
    diFeatures.shaderStorageBufferArrayNonUniformIndexing = true;
    diFeatures.shaderStorageImageArrayNonUniformIndexing = true;
    diFeatures.pNext = &drFeatures;

    auto features = m_physicalDevice.getFeatures2();
    features.pNext = &diFeatures;

    vk::DeviceCreateInfo deviceCi;
    deviceCi.queueCreateInfoCount = queueCreateInfos.size();
    deviceCi.pQueueCreateInfos = queueCreateInfos.data();
    deviceCi.enabledExtensionCount = 0;
    deviceCi.ppEnabledExtensionNames = nullptr;
    deviceCi.pNext = &features;

    auto deviceResult = m_physicalDevice.createDevice(deviceCi);
    MU_CORE_ASSERT(deviceResult, "failed to create device");
    m_device = std::move(*deviceResult);

    std::map<uint32_t, uint32_t> nextQueueIndices;
    nextQueueIndices[graphicsFamily->index] = 0;
    nextQueueIndices[computeFamily->index] = 0;
    nextQueueIndices[transferFamily->index] = 0;

    Queue::Spec graphicsSpec{};
    graphicsSpec.device = &m_device;
    graphicsSpec.queueFamilyIndex = graphicsFamily->index;
    graphicsSpec.queueIndex = nextQueueIndices[graphicsFamily->index]++;
    graphicsSpec.name = "graphics";
    m_graphicsQueue = std::make_unique<Queue>(graphicsSpec);
    MU_CORE_ASSERT(m_graphicsQueue, "graphics queue must not be null");

    Queue::Spec computeSpec{};
    computeSpec.device = &m_device;
    computeSpec.queueFamilyIndex = computeFamily->index;
    computeSpec.queueIndex = nextQueueIndices[computeFamily->index]++;
    computeSpec.name = "compute";
    m_computeQueue = std::make_unique<Queue>(computeSpec);
    MU_CORE_ASSERT(m_computeQueue, "compute queue must not be null");

    Queue::Spec transferSpec{};
    transferSpec.device = &m_device;
    transferSpec.queueFamilyIndex = transferFamily->index;
    transferSpec.queueIndex = nextQueueIndices[transferFamily->index]++;
    transferSpec.name = "transfer";
    m_transferQueue = std::make_unique<Queue>(transferSpec);
    MU_CORE_ASSERT(m_transferQueue, "transfer queue must not be null");
}

auto Context::CreateAllocator() -> void {
    vma::AllocatorCreateInfo createInfo{};
    createInfo.instance = m_instance;
    createInfo.physicalDevice = m_physicalDevice;
    createInfo.device = m_device;
    createInfo.flags = vma::AllocatorCreateFlagBits::eBufferDeviceAddress;

    auto result = vma::createAllocator(createInfo);
    MU_CORE_ASSERT(result.result == vk::Result::eSuccess, "failed to create allocator");

    m_allocator = result.value;
}

} // namespace muon::graphics
