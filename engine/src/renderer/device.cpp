#include "muon/renderer/device.hpp"

#include "muon/core/assert.hpp"
#include "muon/core/log.hpp"
#include "muon/debug/profiler.hpp"
#include "muon/core/window.hpp"
#include <memory>
#include <set>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <vk_mem_alloc.hpp>

#ifdef MU_DEBUG_ENABLED

namespace {

    /**
     * @brief   callback function for Vulkan validation layers.
     *
     * @param   messageSeverity the severity of the message.
     * @param   messageType     what type is the message.
     * @param   callbackData    callback parameters.
     * @param   userData        arbitrary data to be loaded into the callback.
     *
     * @return  false.
     */
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        vk::DebugUtilsMessageTypeFlagsEXT messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT *callbackData,
        void *userData
    ) {
        switch (messageSeverity) {
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            MU_VK_DEBUG(callbackData->pMessage);
            break;

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            MU_VK_INFO(callbackData->pMessage);
            break;

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            MU_VK_WARN(callbackData->pMessage);
            break;

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            MU_VK_ERROR(callbackData->pMessage);
            break;
        }

        return false;
    }

    /**
     * @brief   creates the debug utils messenger.
     *
     * @param   instance        the instance to create debug messenger for.
     * @param   createInfo      what info to create messenger with.
     * @param   allocator       pointer to an allocator.
     * @param   debugMessenger  handle to a debug messenger to be created.
     *
     * @return  result based on the function being executed, otherwise an error.
     */
    vk::Result createDebugUtilsMessenger(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT *createInfo,
        const VkAllocationCallbacks *allocator,
        VkDebugUtilsMessengerEXT *debugMessenger
    ) {
        auto function = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if (function == nullptr) {
            return vk::Result::eErrorExtensionNotPresent;
        }

        return static_cast<vk::Result>(function(instance, createInfo, allocator, debugMessenger));
    }

    /**
     * @brief   destroys the debug utils messenger.
     *
     * @param   instance        the instance to destroy debug messenger for.
     * @param   debugMessenger  handle to a debug messenger to be destroyed.
     * @param   allocator       pointer to an allocator.
     */
    void destroyDebugUtilsMessenger(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *allocator
    ) {
        auto function = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (function == nullptr) {
            return;
        }

        return function(instance, debugMessenger, allocator);
    }

}

#endif

namespace muon {

    Device::Device(Window &window) : m_window(window) {
        createInstance();

        #ifdef MU_DEBUG_ENABLED
        createDebugMessenger();
        #endif

        createSurface();
        selectPhysicalDevice();
        createLogicalDevice();
        createAllocator();
        createCommandPool();

        {
            vk::CommandBufferAllocateInfo allocateInfo{};
            allocateInfo.level = vk::CommandBufferLevel::ePrimary;
            allocateInfo.commandPool = m_commandPool;
            allocateInfo.commandBufferCount = 1;

            vk::CommandBuffer cmd;
            auto result = m_device.allocateCommandBuffers(&allocateInfo, &cmd);
            MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to allocate profiler creation command buffer");

            Profiler::createContext(m_physicalDevice, m_device, m_graphicsQueue, cmd);
        }

        MU_CORE_DEBUG("created device");
    }

    Device::~Device() {
        Profiler::destroyContext();

        m_device.destroyCommandPool(m_commandPool, nullptr);
        m_allocator.destroy();
        m_device.destroy(nullptr);
        m_instance.destroySurfaceKHR(m_surface, nullptr);

        #ifdef MU_DEBUG_ENABLED
        destroyDebugUtilsMessenger(m_instance, m_debugMessenger, nullptr);
        #endif

        m_instance.destroy(nullptr);

        MU_CORE_DEBUG("destroyed device");
    }

    vk::Format Device::findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
        for (auto format : candidates) {
            vk::FormatProperties props;
            m_physicalDevice.getFormatProperties(format, &props);

            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        MU_CORE_ASSERT(false, "failed to find a supported format");
        return vk::Format::eUndefined;
    }

    vk::CommandBuffer Device::beginSingleTimeCommands() {
        vk::CommandBufferAllocateInfo allocateInfo{};
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandPool = m_commandPool;
        allocateInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        auto result = m_device.allocateCommandBuffers(&allocateInfo, &commandBuffer);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to allocate single time command buffer");

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        result = commandBuffer.begin(&beginInfo);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to begin command buffer recording");

        return commandBuffer;
    }

    void Device::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
        commandBuffer.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        auto result = m_graphicsQueue.submit(1, &submitInfo, nullptr);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to submit command buffer to graphics queue");

        m_graphicsQueue.waitIdle();

        m_device.freeCommandBuffers(m_commandPool, 1, &commandBuffer);
    }

    void Device::copyBuffer(vk::Buffer src, vk::Buffer dest, vk::DeviceSize size) {
        vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

        vk::BufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        commandBuffer.copyBuffer(src, dest, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void Device::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount) {
        vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

        vk::BufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.setImageOffset({0, 0, 0});
        region.setImageExtent({width, height, 1});

        commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

        endSingleTimeCommands(commandBuffer);
    }

    void Device::copyImageToBuffer(vk::Image image, vk::Buffer buffer, uint32_t width, uint32_t height, uint32_t layerCount) {
        vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

        vk::BufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layerCount;

        region.imageOffset.x = 0;
        region.imageOffset.y = 0;
        region.imageOffset.z = 0;

        region.imageExtent.width = width;
        region.imageExtent.height = height;
        region.imageExtent.depth = 1;

        commandBuffer.copyImageToBuffer(image, vk::ImageLayout::eTransferSrcOptimal, buffer, 1, &region);

        endSingleTimeCommands(commandBuffer);
    }

    void Device::createImage(const vk::ImageCreateInfo &imageInfo, vk::MemoryPropertyFlags properties, vma::MemoryUsage memoryUsage, vk::Image &image, vma::Allocation &allocation) {
        auto result = m_device.createImage(&imageInfo, nullptr, &image);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create an image");

        vk::MemoryRequirements memoryRequirements{};
        m_device.getImageMemoryRequirements(image, &memoryRequirements);

        vma::AllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = memoryUsage;
        allocCreateInfo.requiredFlags = properties;

        vma::AllocationInfo allocationInfo;

        result = m_allocator.allocateMemory(&memoryRequirements, &allocCreateInfo, &allocation, &allocationInfo);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to allocate image memory");

        m_allocator.bindImageMemory(allocation, image);
    }

    void Device::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vma::MemoryUsage memoryUsage, vk::Buffer &buffer, vma::Allocation &allocation) {
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        vma::AllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = memoryUsage;

        vma::AllocationInfo allocationInfo;

        auto result = m_allocator.createBuffer(&bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocationInfo);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create buffer");
    }

    vk::Instance Device::instance() const {
        return m_instance;
    }

    vk::SurfaceKHR Device::surface() const {
        return m_surface;
    }

    vk::PhysicalDevice Device::physicalDevice() const {
        return m_physicalDevice;
    }

    vk::Device Device::device() const {
        return m_device;
    }

    vk::CommandPool Device::commandPool() const {
        return m_commandPool;
    }

    vk::Queue Device::graphicsQueue() const {
        return m_graphicsQueue;
    }

    vk::Queue Device::computeQueue() const {
        return m_computeQueue;
    }

    vk::Queue Device::presentQueue() const {
        return m_presentQueue;
    }

    vma::Allocator Device::allocator() const {
        return m_allocator;
    }

    std::unique_ptr<QueueFamilyIndices> &Device::queueFamilyIndices() {
        return m_queueFamilyIndices;
    }

    SwapchainSupportDetails Device::swapchainSupportDetails() {
        return querySwapchainSupportDetails(m_physicalDevice);
    }

    void Device::createInstance() {
        #ifdef MUON_DEBUG_ENABLED
        auto checkValidationLayerSupport = [this]() -> bool {
            std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

            for (const auto layerName : validationLayers) {
                bool layerFound = false;
                for (const auto &layerProperties : availableLayers) {
                    if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    return false;
                }
            }

            return true;
        };

        MU_CORE_ASSERT(checkValidationLayerSupport(), "validation layers were requested but are not available");
        #endif

        // auto getRequiredExtensions = []() -> std::vector<const char *> {
        //     uint32_t sdlExtensionCount = 0;
        //     const char *const *sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

        //     MU_CORE_ASSERT(sdlExtensions != nullptr, "SDL must provide extensions");

        //     std::vector<const char *> extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);

        //     #ifdef MU_DEBUG_ENABLED
        //     extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        //     #endif

        //     return extensions;
        // };
        auto extensions = m_window.requiredExtensions();
        #ifdef MU_DEBUG_ENABLED
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        #endif

        vk::ApplicationInfo appInfo;
        appInfo.pApplicationName = "Muon";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Muon";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.pNext = nullptr;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        #ifdef MU_DEBUG_ENABLED
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
        createInfo.ppEnabledLayerNames = m_validationLayers.data();
        #endif

        auto result = vk::createInstance(&createInfo, nullptr, &m_instance);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create instance");
    }

    void Device::createDebugMessenger() {
        vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.messageSeverity =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        createInfo.messageType =
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
        createInfo.pfnUserCallback = debugCallback;

        auto result = createDebugUtilsMessenger(
            m_instance,
            reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT *>(&createInfo),
            nullptr,
            reinterpret_cast<VkDebugUtilsMessengerEXT *>(&m_debugMessenger)
        );
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to debug messenger");
    }

    void Device::createSurface() {
        // auto success = m_window.createSurface(m_instance, reinterpret_cast<VkSurfaceKHR *>(&m_surface));
        // MU_CORE_ASSERT(success, std::format("failed to create window surface: {}", SDL_GetError()));
        auto result = m_window.createSurface(m_instance, &m_surface);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create window surface");
    }

    void Device::selectPhysicalDevice() {
        std::vector<vk::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices();
        MU_CORE_ASSERT(physicalDevices.size() > 0, "no GPUs available with Vulkan support");

        auto checkDeviceExtensionSupport = [&](vk::PhysicalDevice device) -> bool {
            std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

            std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

            for (const auto &extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        };

        auto isDeviceSuitable = [&](vk::PhysicalDevice physicalDevice) -> bool {
            vk::PhysicalDeviceProperties2 deviceProperties = physicalDevice.getProperties2();
            vk::PhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
            vk::PhysicalDeviceFeatures2 deviceFeatures{};
            deviceFeatures.pNext = &indexingFeatures;
            physicalDevice.getFeatures2(&deviceFeatures);

            bool isDiscrete = deviceProperties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
            findQueueFamilies(physicalDevice);
            bool hasCompleteIndices = m_queueFamilyIndices->isComplete();
            bool supportsGeometryShader = deviceFeatures.features.geometryShader;
            bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);
            bool swapchainAdequate = false;
            if (extensionsSupported) {
                auto details = querySwapchainSupportDetails(physicalDevice);
                swapchainAdequate = !details.formats.empty() && !details.presentModes.empty();
            }
            bool supportsBindless = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;

            return isDiscrete && hasCompleteIndices && supportsGeometryShader && extensionsSupported && swapchainAdequate && supportsBindless;
        };

        for (const auto &physicalDevice : physicalDevices) {
            if (isDeviceSuitable(physicalDevice)) {
                m_physicalDevice = physicalDevice;
                break;
            }
        }
        MU_CORE_ASSERT(m_physicalDevice, "unable to select a suitable GPU");

        findQueueFamilies(m_physicalDevice);
        MU_CORE_ASSERT(m_queueFamilyIndices->isComplete(), "queue family indices are not complete");
    }

    void Device::createLogicalDevice() {
        std::set<uint32_t> uniqueQueueFamilies = {
            *m_queueFamilyIndices->graphicsFamily,
            *m_queueFamilyIndices->computeFamily,
            *m_queueFamilyIndices->presentFamily
        };
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilies.size());

        float queuePriority = 1.0;

        size_t index{0};
        for (auto queueFamily : uniqueQueueFamilies) {
            vk::DeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos[index] = queueCreateInfo;
            index += 1;
        }

        vk::PhysicalDeviceSynchronization2Features syncFeatures{};
        syncFeatures.synchronization2 = true;

        vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
        dynamicRenderingFeatures.pNext = &syncFeatures;
        dynamicRenderingFeatures.dynamicRendering = true;

        vk::PhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
        indexingFeatures.pNext = &dynamicRenderingFeatures;

        vk::PhysicalDeviceFeatures2 deviceFeatures{};
        deviceFeatures.pNext = &indexingFeatures;
        m_physicalDevice.getFeatures2(&deviceFeatures);

        vk::DeviceCreateInfo createInfo{};
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();
        createInfo.pNext = &deviceFeatures;

        auto result = m_physicalDevice.createDevice(&createInfo, nullptr, &m_device);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create a logical device");

        m_device.getQueue(*m_queueFamilyIndices->graphicsFamily, 0, &m_graphicsQueue);
        m_device.getQueue(*m_queueFamilyIndices->computeFamily, 0, &m_computeQueue);
        m_device.getQueue(*m_queueFamilyIndices->presentFamily, 0, &m_presentQueue);
    }

    void Device::createAllocator() {
        vma::AllocatorCreateInfo allocatorInfo{};
        allocatorInfo.instance = m_instance;
        allocatorInfo.physicalDevice = m_physicalDevice;
        allocatorInfo.device = m_device;

        auto result = vma::createAllocator(&allocatorInfo, &m_allocator);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create allocator");
    }

    void Device::createCommandPool() {
        auto &indices = m_queueFamilyIndices;

        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.queueFamilyIndex = *indices->graphicsFamily;
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        auto result = m_device.createCommandPool(&poolInfo, nullptr, &m_commandPool);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to create command pool");
    }

    void Device::findQueueFamilies(vk::PhysicalDevice physicalDevice) {
        m_queueFamilyIndices = std::make_unique<QueueFamilyIndices>();

        std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

        uint32_t i = 0;
        for (const auto &queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                m_queueFamilyIndices->graphicsFamily = i;
            }

            if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
                m_queueFamilyIndices->computeFamily = i;
            }

            vk::Bool32 presentSupport = false;
            auto result = physicalDevice.getSurfaceSupportKHR(i, m_surface, &presentSupport);

            if (presentSupport && result == vk::Result::eSuccess) {
                m_queueFamilyIndices->presentFamily = i;
            }

            if (m_queueFamilyIndices->isComplete()) {
                break;
            }

            i++;
        }
    }

    SwapchainSupportDetails Device::querySwapchainSupportDetails(vk::PhysicalDevice physicalDevice) {
        SwapchainSupportDetails details{};

        vk::Result result;

        result = physicalDevice.getSurfaceCapabilitiesKHR(m_surface, &details.capabilities);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to get surface capabilities");

        uint32_t formatCount{0};
        result = physicalDevice.getSurfaceFormatsKHR(m_surface, &formatCount, nullptr);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to get surface formats");

        MU_CORE_ASSERT(formatCount > 0, "there must be more than 0 surface formats");
        details.formats.resize(formatCount);

        result = physicalDevice.getSurfaceFormatsKHR(m_surface, &formatCount, details.formats.data());
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to get surface formats");

        uint32_t presentModeCount;
        result = physicalDevice.getSurfacePresentModesKHR(m_surface, &presentModeCount, nullptr);
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to get surface present modes");

        MU_CORE_ASSERT(presentModeCount > 0, "there must be more than 0 present modes");
        details.presentModes.resize(presentModeCount);

        result = physicalDevice.getSurfacePresentModesKHR(m_surface, &presentModeCount, details.presentModes.data());
        MU_CORE_ASSERT(result == vk::Result::eSuccess, "failed to get surface present modes");

        return details;
    }

}
