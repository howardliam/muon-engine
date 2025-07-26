#include "muon/graphics/context.hpp"

#include "muon/core/application.hpp"
#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/graphics/api.hpp"
#include "muon/graphics/extensions.hpp"
#include "muon/graphics/gpu.hpp"
#include "muon/graphics/queue.hpp"
#include "muon/graphics/queue_info.hpp"
#include "vulkan/vulkan.hpp"

#include <numeric>
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

auto Context::getVramUsage() const -> uint64_t {
    auto chain =
        m_physicalDevice.getMemoryProperties2<vk::PhysicalDeviceMemoryProperties2, vk::PhysicalDeviceMemoryBudgetPropertiesEXT>();

    auto memoryBudgetProps = chain.get<vk::PhysicalDeviceMemoryBudgetPropertiesEXT>();

    return std::accumulate(memoryBudgetProps.heapUsage.begin(), memoryBudgetProps.heapUsage.end(), 0);
}
auto Context::getVramCapacity() const -> uint64_t { return m_vramCapacity; }

auto Context::getInstance() -> vk::raii::Instance & { return m_instance; }
auto Context::getInstance() const -> const vk::raii::Instance & { return m_instance; }

auto Context::getSurface() -> vk::raii::SurfaceKHR & { return m_surface; }
auto Context::getSurface() const -> const vk::raii::SurfaceKHR & { return m_surface; }

auto Context::getPhysicalDevice() -> vk::raii::PhysicalDevice & { return m_physicalDevice; }
auto Context::getPhysicalDevice() const -> const vk::raii::PhysicalDevice & { return m_physicalDevice; }

auto Context::getDevice() -> vk::raii::Device & { return m_device; }
auto Context::getDevice() const -> const vk::raii::Device & { return m_device; }

auto Context::getGraphicsQueue() -> Queue & { return *m_graphicsQueue; }
auto Context::getGraphicsQueue() const -> const Queue & { return *m_graphicsQueue; }

auto Context::getComputeQueue() -> Queue & { return *m_computeQueue; }
auto Context::getComputeQueue() const -> const Queue & { return *m_computeQueue; }

auto Context::getTransferQueue() -> Queue & { return *m_transferQueue; }
auto Context::getTransferQueue() const -> const Queue & { return *m_transferQueue; }

auto Context::getAllocator() -> vma::Allocator & { return m_allocator; }
auto Context::getAllocator() const -> const vma::Allocator & { return m_allocator; }

auto Context::createInstance(const Window &window, bool debug) -> void {
    std::vector extensions = window.getRequiredExtensions();
    extensions.insert(extensions.end(), k_instanceRequiredExtensions.begin(), k_instanceRequiredExtensions.end());

    std::vector<const char *> layers;

    if (debug) {
        extensions.push_back("VK_EXT_debug_utils");

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
            layers.push_back(validationLayer);
        } else {
            core::warn("`{}` is unavailable", validationLayer);
        }
    }

    vk::ApplicationInfo appInfo;
    appInfo.apiVersion = k_vulkanApiVersion;

    appInfo.pApplicationName = Application::get().getName().data();
    appInfo.applicationVersion = vk::makeVersion(0, 1, 0);

    appInfo.pEngineName = "Muon";
    appInfo.engineVersion = vk::makeVersion(0, 1, 0);

    vk::InstanceCreateInfo instanceCi;
    instanceCi.enabledExtensionCount = extensions.size();
    instanceCi.ppEnabledExtensionNames = extensions.data();

    instanceCi.enabledLayerCount = layers.size();
    instanceCi.ppEnabledLayerNames = layers.data();

    instanceCi.pApplicationInfo = &appInfo;

    auto instanceResult = m_context.createInstance(instanceCi);
    core::expect(instanceResult, "failed to create instance");
    m_instance = std::move(*instanceResult);
}

auto Context::createDebugMessenger(bool debug) -> void {
    if (!debug) {
        return;
    }

    auto debugCallback = [](vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type,
                            const vk::DebugUtilsMessengerCallbackDataEXT *callbackData, void *userData) -> vk::Bool32 {
        switch (severity) {
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
    };

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

    std::vector<Gpu> gpus;

    for (const auto &physicalDevice : physicalDevices) {
        gpuSpec.physicalDevice = &physicalDevice;
        Gpu gpu{gpuSpec};

        if (gpu.isSuitable()) {
            gpus.push_back(gpu);
        }
    }

    std::sort(gpus.begin(), gpus.end(), [](const Gpu &lhs, const Gpu &rhs) -> bool {
        return lhs.getMemorySize() > rhs.getMemorySize();
    });

    bool gpuSelected = false;

    if (gpus.size() >= 1) {
        auto &gpu = gpus.front();
        m_physicalDevice = gpu.getPhysicalDevice();
        m_vramCapacity = gpu.getMemorySize();
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

    auto features = vk::StructureChain<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan12Features, vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceVulkan14Features, vk::PhysicalDeviceMeshShaderFeaturesEXT,
        vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT, vk::PhysicalDeviceVertexInputDynamicStateFeaturesEXT>{};

    auto &vidsFeatures = features.get<vk::PhysicalDeviceVertexInputDynamicStateFeaturesEXT>();
    vidsFeatures.vertexInputDynamicState = true;

    auto &ds3Features = features.get<vk::PhysicalDeviceExtendedDynamicState3FeaturesEXT>();
    ds3Features.extendedDynamicState3PolygonMode = true;

    auto &msFeatures = features.get<vk::PhysicalDeviceMeshShaderFeaturesEXT>();
    msFeatures.meshShader = true;
    msFeatures.taskShader = true;

    auto &vk14Features = features.get<vk::PhysicalDeviceVulkan14Features>();
    vk14Features.pushDescriptor = true;

    auto &vk13Features = features.get<vk::PhysicalDeviceVulkan13Features>();
    vk13Features.synchronization2 = true;
    vk13Features.dynamicRendering = true;

    auto &vk12Features = features.get<vk::PhysicalDeviceVulkan12Features>();
    vk12Features.bufferDeviceAddress = true;
    vk12Features.timelineSemaphore = true;
    vk12Features.scalarBlockLayout = true;

    vk12Features.shaderInputAttachmentArrayDynamicIndexing = true;
    vk12Features.shaderUniformTexelBufferArrayDynamicIndexing = true;
    vk12Features.shaderStorageTexelBufferArrayDynamicIndexing = true;
    vk12Features.shaderUniformBufferArrayNonUniformIndexing = true;
    vk12Features.shaderSampledImageArrayNonUniformIndexing = true;
    vk12Features.shaderStorageBufferArrayNonUniformIndexing = true;
    vk12Features.shaderStorageImageArrayNonUniformIndexing = true;
    vk12Features.shaderInputAttachmentArrayNonUniformIndexing = true;
    vk12Features.shaderUniformTexelBufferArrayNonUniformIndexing = true;
    vk12Features.shaderStorageTexelBufferArrayNonUniformIndexing = true;
    vk12Features.descriptorBindingUniformBufferUpdateAfterBind = true;
    vk12Features.descriptorBindingSampledImageUpdateAfterBind = true;
    vk12Features.descriptorBindingStorageImageUpdateAfterBind = true;
    vk12Features.descriptorBindingStorageBufferUpdateAfterBind = true;
    vk12Features.descriptorBindingUniformTexelBufferUpdateAfterBind = true;
    vk12Features.descriptorBindingStorageTexelBufferUpdateAfterBind = true;
    vk12Features.descriptorBindingUpdateUnusedWhilePending = true;
    vk12Features.descriptorBindingPartiallyBound = true;
    vk12Features.descriptorBindingVariableDescriptorCount = true;
    vk12Features.runtimeDescriptorArray = true;

    auto &deviceFeatures = features.get<vk::PhysicalDeviceFeatures2>().features;
    deviceFeatures = m_physicalDevice.getFeatures();

    vk::DeviceCreateInfo deviceCi;
    deviceCi.queueCreateInfoCount = queueCreateInfos.size();
    deviceCi.pQueueCreateInfos = queueCreateInfos.data();
    deviceCi.enabledExtensionCount = k_deviceRequiredExtensions.size();
    deviceCi.ppEnabledExtensionNames = k_deviceRequiredExtensions.data();
    deviceCi.pNext = &features.get<vk::PhysicalDeviceFeatures2>();

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

    auto [result, allocator] = vma::createAllocator(createInfo);
    core::expect(result == vk::Result::eSuccess, "failed to create allocator");
    m_allocator = allocator;
}

} // namespace muon::graphics
