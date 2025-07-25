#include "muon/graphics/context.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/graphics/extensions.hpp"
#include "muon/graphics/gpu.hpp"
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

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageType,
    const vk::DebugUtilsMessengerCallbackDataEXT *callbackData, void *userData
) {
    switch (messageSeverity) {
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            muon::core::debug(callbackData->pMessage);
            break;

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            muon::core::info(callbackData->pMessage);
            break;

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            muon::core::warn(callbackData->pMessage);
            break;

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            muon::core::error(callbackData->pMessage);
            break;

        default:
            break;
    }

    return false;
}

namespace muon::graphics {

Context::Context(const Spec &spec) {
    core::expect(spec.window, "a window must be present");

    m_context = vk::raii::Context();

    createInstance(*spec.window, spec.debug);
    createDebugMessenger(spec.debug);
    createSurface(*spec.window);
    selectPhysicalDevice();
    createLogicalDevice();
    createAllocator();

    core::debug("created device");
}

Context::~Context() {
    m_allocator.destroy();
    m_graphicsQueue.reset();
    m_computeQueue.reset();
    m_transferQueue.reset();

    core::debug("destroyed device");
}

auto Context::getInstance() -> vk::raii::Instance & { return m_instance; }
auto Context::getInstance() const -> const vk::raii::Instance & { return m_instance; }

auto Context::getSurface() -> vk::raii::SurfaceKHR & { return m_surface; }
auto Context::getSurface() const -> const vk::raii::SurfaceKHR & { return m_surface; }

auto Context::getPhysicalDevice() -> vk::raii::PhysicalDevice & { return m_physicalDevice; }
auto Context::getPhysicalDevice() const -> const vk::raii::PhysicalDevice & { return m_physicalDevice; }

auto Context::getDevice() -> vk::raii::Device & { return m_device; }
auto Context::getDevice() const -> const vk::raii::Device & { return m_device; }

auto Context::getGraphicsQueue() const -> Queue & { return *m_graphicsQueue; }
auto Context::getComputeQueue() const -> Queue & { return *m_computeQueue; }
auto Context::getTransferQueue() const -> Queue & { return *m_transferQueue; }

auto Context::getAllocator() const -> vma::Allocator { return m_allocator; }

auto Context::createInstance(const Window &window, bool debug) -> void {
    auto extensions = window.getRequiredExtensions();
    extensions.insert(extensions.end(), k_instanceRequiredExtensions.begin(), k_instanceRequiredExtensions.end());

    if (debug) {
        extensions.push_back("VK_EXT_debug_utils");
    }

    vk::ApplicationInfo appInfo;
    appInfo.pApplicationName = "Muon";
    appInfo.applicationVersion = vk::makeVersion(0, 1, 0);
    appInfo.pEngineName = "Muon";
    appInfo.engineVersion = vk::makeVersion(0, 1, 0);
    appInfo.apiVersion = vk::ApiVersion13;

    vk::InstanceCreateInfo instanceCi;
    instanceCi.pApplicationInfo = &appInfo;
    instanceCi.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceCi.ppEnabledExtensionNames = extensions.data();

    if (debug) {
        auto checkValidationLayerSupport = [](const vk::raii::Context &context, const char *layer) -> bool {
            auto availableLayers = context.enumerateInstanceLayerProperties();

            auto it = std::ranges::find_if(availableLayers, [&](const VkLayerProperties &props) -> bool {
                return std::strcmp(props.layerName, layer) == 0;
            });

            if (it == availableLayers.end()) {
                return false;
            }

            return true;
        };

        const char *validationLayer = "VK_LAYER_KHRONOS_validation";

        if (checkValidationLayerSupport(m_context, validationLayer)) {
            instanceCi.enabledLayerCount = 1;
            instanceCi.ppEnabledLayerNames = &validationLayer;
        } else {
            core::warn("the validation layer is not available");
        }
    }

    auto instanceResult = m_context.createInstance(instanceCi);
    core::expect(instanceResult, "failed to create instance");
    m_instance = std::move(*instanceResult);
}

auto Context::createDebugMessenger(bool debug) -> void {
    if (!debug) {
        return;
    }

    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCi;
    debugUtilsMessengerCi.pfnUserCallback = debugCallback;

    debugUtilsMessengerCi.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

    debugUtilsMessengerCi.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

    auto debugMessengerResult = m_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCi);
    core::expect(debugMessengerResult, "failed to create debug messenger");

    m_debugMessenger = std::move(*debugMessengerResult);
}

auto Context::createSurface(const Window &window) -> void {
    auto surfaceResult = window.createSurface(m_instance);
    core::expect(surfaceResult, "failed to create window surface");
    m_surface = std::move(*surfaceResult);
    core::expect(*m_surface, "surface must not be null");
}

auto Context::selectPhysicalDevice() -> void {
    auto physicalDevicesResult = m_instance.enumeratePhysicalDevices();
    core::expect(physicalDevicesResult, "failed to get available GPUs");
    auto physicalDevices = *physicalDevicesResult;

    Gpu::Spec gpuSpec{};
    gpuSpec.surface = &m_surface;

    using GpuPair = std::pair<Gpu, const vk::raii::PhysicalDevice *>;
    std::vector<GpuPair> gpus{};

    for (const auto &physicalDevice : physicalDevices) {
        gpuSpec.physicalDevice = &physicalDevice;
        Gpu gpu(gpuSpec);

        if (gpu.isSuitable()) {
            gpus.push_back({gpu, &physicalDevice});
        }
    }

    auto sort = [](const GpuPair &a, const GpuPair &b) -> bool { return a.first.getMemorySize() > b.first.getMemorySize(); };
    std::sort(gpus.begin(), gpus.end(), sort);

    bool gpuSelected = false;

    if (gpus.size() >= 1) {
        auto &[gpu, physicalDevice] = gpus.front();
        m_physicalDevice = *physicalDevice;
        gpuSelected = true;
    }

    core::expect(gpuSelected, "failed to select a suitable GPU");
}

auto Context::createLogicalDevice() -> void {
    const QueueInfo queueInfo{m_physicalDevice, m_surface};
    core::expect(queueInfo.getFamilyInfo().size() >= 1, "there must be at least one queue family");
    core::expect(queueInfo.getTotalQueueCount() >= 3, "there must be at least three queues available");

    const auto queueFamilies = queueInfo.getFamilyInfo();

    auto graphicsFamily =
        std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) { return info.isGraphicsCapable(); });
    core::expect(graphicsFamily != queueFamilies.end(), "there must be a graphics capable queue family");
    core::expect(graphicsFamily->isPresentCapable(), "the graphics capable queue family must support presentation");

    auto computeFamily =
        std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) { return info.isComputeDedicated(); });
    if (computeFamily == queueFamilies.end()) {
        computeFamily = std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) {
            return info.isComputeCapable() && info.queueCount > 1;
        });
    }
    core::expect(computeFamily != queueFamilies.end(), "there must be a compute capable queue family");

    auto transferFamily =
        std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) { return info.isTransferDedicated(); });
    if (transferFamily == queueFamilies.end()) {
        transferFamily = std::ranges::find_if(queueFamilies, [](const QueueFamilyInfo &info) {
            return info.isTransferCapable() && info.queueCount > 1;
        });
    }
    core::expect(transferFamily != queueFamilies.end(), "there must be a transfer capable queue family");

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
    deviceCi.enabledExtensionCount = k_deviceRequiredExtensions.size();
    deviceCi.ppEnabledExtensionNames = k_deviceRequiredExtensions.data();
    deviceCi.pNext = &features;

    auto deviceResult = m_physicalDevice.createDevice(deviceCi);
    core::expect(deviceResult, "failed to create device");
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
    core::expect(m_graphicsQueue, "graphics queue must not be null");

    Queue::Spec computeSpec{};
    computeSpec.device = &m_device;
    computeSpec.queueFamilyIndex = computeFamily->index;
    computeSpec.queueIndex = nextQueueIndices[computeFamily->index]++;
    computeSpec.name = "compute";
    m_computeQueue = std::make_unique<Queue>(computeSpec);
    core::expect(m_computeQueue, "compute queue must not be null");

    Queue::Spec transferSpec{};
    transferSpec.device = &m_device;
    transferSpec.queueFamilyIndex = transferFamily->index;
    transferSpec.queueIndex = nextQueueIndices[transferFamily->index]++;
    transferSpec.name = "transfer";
    m_transferQueue = std::make_unique<Queue>(transferSpec);
    core::expect(m_transferQueue, "transfer queue must not be null");
}

auto Context::createAllocator() -> void {
    vma::AllocatorCreateInfo createInfo{};
    createInfo.instance = m_instance;
    createInfo.physicalDevice = m_physicalDevice;
    createInfo.device = m_device;
    createInfo.flags = vma::AllocatorCreateFlagBits::eBufferDeviceAddress;

    auto result = vma::createAllocator(createInfo);
    core::expect(result.result == vk::Result::eSuccess, "failed to create allocator");

    m_allocator = result.value;
}

} // namespace muon::graphics
