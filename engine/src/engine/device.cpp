#include "muon/engine/device.hpp"

#include "muon/log/logger.hpp"
#include "muon/engine/window.hpp"
#include <SDL3/SDL_vulkan.h>
#include <format>
#include <set>
#include <stdexcept>
#include <ranges>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <vk_mem_alloc.hpp>

namespace muon::engine {

    #ifndef NDEBUG
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
            log::globalLogger->debug("VULKAN - \n{}\n", callbackData->pMessage);
            break;

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            log::globalLogger->info("VULKAN - \n{}\n", callbackData->pMessage);
            break;

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            log::globalLogger->warn("VULKAN - \n{}\n", callbackData->pMessage);
            break;

        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            log::globalLogger->error("VULKAN - \n{}\n", callbackData->pMessage);
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
    #endif

    Device::Device(Window &window) : window(window) {
        createInstance();

        #ifndef NDEBUG
        createDebugMessenger();
        #endif

        createSurface();
        selectPhysicalDevice();
        createLogicalDevice();
        createAllocator();
        createCommandPool();

        log::globalLogger->debug("created device");
    }

    Device::~Device() {
        allocator.destroy();
        device.destroyCommandPool(commandPool, nullptr);
        device.destroy(nullptr);
        instance.destroySurfaceKHR(surface, nullptr);

        #ifndef NDEBUG
        destroyDebugUtilsMessenger(instance, debugMessenger, nullptr);
        #endif

        instance.destroy(nullptr);

        log::globalLogger->debug("destroyed device");
    }

    vk::Format Device::findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
        for (auto format : candidates) {
            vk::FormatProperties props;
            physicalDevice.getFormatProperties(format, &props);

            if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find a supported format");
    }

    vk::CommandBuffer Device::beginSingleTimeCommands() {
        vk::CommandBufferAllocateInfo allocateInfo;
        allocateInfo.level = vk::CommandBufferLevel::ePrimary;
        allocateInfo.commandPool = commandPool;
        allocateInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        auto result = device.allocateCommandBuffers(&allocateInfo, &commandBuffer);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to allocate single time command buffer");
        }

        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        result = commandBuffer.begin(&beginInfo);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to begin command buffer recording");
        }

        return commandBuffer;
    }

    void Device::endSingleTimeCommands(vk::CommandBuffer commandBuffer) {
        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        auto result = graphicsQueue.submit(1, &submitInfo, nullptr);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to submit command buffer to graphics queue");
        }

        graphicsQueue.waitIdle();

        device.freeCommandBuffers(commandPool, 1, &commandBuffer);
    }

    void Device::copyBuffer(vk::Buffer src, vk::Buffer dest, vk::DeviceSize size) {
        vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

        vk::BufferCopy copyRegion;
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;

        commandBuffer.copyBuffer(src, dest, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void Device::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount) {
        vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

        vk::BufferImageCopy region;
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
        auto result = device.createImage(&imageInfo, nullptr, &image);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create an image");
        }

        vk::MemoryRequirements memoryRequirements{};
        device.getImageMemoryRequirements(image, &memoryRequirements);

        vma::AllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = memoryUsage;
        allocCreateInfo.requiredFlags = properties;

        vma::AllocationInfo allocationInfo;

        result = allocator.allocateMemory(&memoryRequirements, &allocCreateInfo, &allocation, &allocationInfo);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to allocate image memory");
        }

        allocator.bindImageMemory(allocation, image);
    }

    void Device::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vma::MemoryUsage memoryUsage, vk::Buffer &buffer, vma::Allocation &allocation) {
        vk::BufferCreateInfo bufferInfo{};
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = vk::SharingMode::eExclusive;

        vma::AllocationCreateInfo allocCreateInfo{};
        allocCreateInfo.usage = memoryUsage;

        vma::AllocationInfo allocationInfo;

        auto result = allocator.createBuffer(&bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocationInfo);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create buffer");
        }
    }

    vk::Instance Device::getInstance() const {
        return instance;
    }

    vk::SurfaceKHR Device::getSurface() const {
        return surface;
    }

    vk::PhysicalDevice Device::getPhysicalDevice() const {
        return physicalDevice;
    }

    vk::Device Device::getDevice() const {
        return device;
    }

    vk::CommandPool Device::getCommandPool() const {
        return commandPool;
    }

    vk::Queue Device::getGraphicsQueue() const {
        return graphicsQueue;
    }

    vk::Queue Device::getComputeQueue() const {
        return computeQueue;
    }

    vk::Queue Device::getPresentQueue() const {
        return presentQueue;
    }

    vma::Allocator Device::getAllocator() const {
        return allocator;
    }

    QueueFamilyIndices Device::getQueueFamilyIndices() {
        return findQueueFamilies(physicalDevice);
    }

    SwapchainSupportDetails Device::getSwapchainSupportDetails() {
        return querySwapchainSupport(physicalDevice);
    }

    void Device::createInstance() {
        #ifndef NDEBUG
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

        if (!checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers were requested but are not available");
        }
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

        auto getRequiredExtensions = []() -> std::vector<const char *> {
            uint32_t sdlExtensionCount = 0;
            const char *const *sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

            if (sdlExtensions == nullptr) {
                throw std::runtime_error(std::format("failed to get instance extensions from SDL: {}", SDL_GetError()));
            }

            std::vector<const char *> extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);

            #ifndef NDEBUG
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            #endif

            return extensions;
        };

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        #ifndef NDEBUG
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        #endif

        auto result = vk::createInstance(&createInfo, nullptr, &instance);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create instance");
        }
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
            instance,
            reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT *>(&createInfo),
            nullptr,
            reinterpret_cast<VkDebugUtilsMessengerEXT *>(&debugMessenger)
        );

        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create debug messenger");
        }
    }

    void Device::createSurface() {
        auto success = window.createSurface(instance, reinterpret_cast<VkSurfaceKHR *>(&surface));
        if (!success) {
            throw std::runtime_error(std::format("failed to create window surface: {}", SDL_GetError()));
        }
    }

    void Device::selectPhysicalDevice() {
        std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
        if (physicalDevices.size() == 0) {
            throw std::runtime_error("no GPUs available with Vulkan support");
        }

        auto checkDeviceExtensionSupport = [&](vk::PhysicalDevice device) -> bool {
            std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

            for (const auto &extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        };

        auto isDeviceSuitable = [&](vk::PhysicalDevice device) -> bool {
            vk::PhysicalDeviceProperties2 deviceProperties = device.getProperties2();
            vk::PhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
            vk::PhysicalDeviceFeatures2 deviceFeatures{};
            deviceFeatures.pNext = &indexingFeatures;
            device.getFeatures2(&deviceFeatures);

            bool isDiscrete = deviceProperties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
            bool hasCompleteIndices = findQueueFamilies(device).isComplete();
            bool supportsGeometryShader = deviceFeatures.features.geometryShader;
            bool extensionsSupported = checkDeviceExtensionSupport(device);
            bool swapchainAdequate = false;
            if (extensionsSupported) {
                SwapchainSupportDetails supportDetails = querySwapchainSupport(device);
                swapchainAdequate = !supportDetails.formats.empty() && !supportDetails.presentModes.empty();
            }
            bool supportsBindless = indexingFeatures.descriptorBindingPartiallyBound && indexingFeatures.runtimeDescriptorArray;


            return isDiscrete && hasCompleteIndices && supportsGeometryShader && extensionsSupported && swapchainAdequate && supportsBindless;
        };

        for (const auto &pd : physicalDevices) {
            if (isDeviceSuitable(pd)) {
                physicalDevice = pd;
                break;
            }
        }

        if (physicalDevice == nullptr) {
            throw std::runtime_error("unable to select a suitable GPU");
        }
    }

    void Device::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        if (!indices.isComplete()) {
            throw std::runtime_error("queue family indices are not complete");
        }

        std::set<uint32_t> uniqueQueueFamilies = {
            *indices.graphicsFamily,
            *indices.computeFamily,
            *indices.presentFamily
        };
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos(uniqueQueueFamilies.size());

        float queuePriority = 1.0;

        for (auto [index, queueFamily] : std::views::enumerate(uniqueQueueFamilies)) {
            vk::DeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos[index] = queueCreateInfo;
        }


        vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
        dynamicRenderingFeatures.pNext = nullptr;
        dynamicRenderingFeatures.dynamicRendering = true;

        vk::PhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
        indexingFeatures.pNext = &dynamicRenderingFeatures;

        vk::PhysicalDeviceFeatures2 deviceFeatures{};
        deviceFeatures.pNext = &indexingFeatures;
        physicalDevice.getFeatures2(&deviceFeatures);

        vk::DeviceCreateInfo createInfo{};
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        createInfo.pNext = &deviceFeatures;

        #ifndef NDEBUG
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        #endif

        auto result = physicalDevice.createDevice(&createInfo, nullptr, &device);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create a logical device");
        }

        device.getQueue(*indices.graphicsFamily, 0, &graphicsQueue);
        device.getQueue(*indices.computeFamily, 0, &computeQueue);
        device.getQueue(*indices.presentFamily, 0, &presentQueue);
    }

    void Device::createAllocator() {
        vma::AllocatorCreateInfo allocatorInfo{};
        allocatorInfo.instance = instance;
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;

        auto result = vma::createAllocator(&allocatorInfo, &allocator);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create allocator");
        }
    }

    void Device::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = getQueueFamilyIndices();

        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.queueFamilyIndex = *queueFamilyIndices.graphicsFamily;
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        auto result = device.createCommandPool(&poolInfo, nullptr, &commandPool);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create command pool");
        }
    }

    QueueFamilyIndices Device::findQueueFamilies(vk::PhysicalDevice physicalDevice) {
        QueueFamilyIndices indices;

        std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();

        uint32_t i = 0;
        for (const auto &queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
                indices.graphicsFamily = i;
            }

            if (queueFamily.queueFlags & vk::QueueFlagBits::eCompute) {
                indices.computeFamily = i;
            }

            vk::Bool32 presentSupport = false;
            auto result = physicalDevice.getSurfaceSupportKHR(i, surface, &presentSupport);

            if (presentSupport && result == vk::Result::eSuccess) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    SwapchainSupportDetails Device::querySwapchainSupport(vk::PhysicalDevice physicalDevice) {
        SwapchainSupportDetails details{};

        auto result = physicalDevice.getSurfaceCapabilitiesKHR(surface, &details.capabilities);
        if (result != vk::Result::eSuccess) {
            log::globalLogger->error("failed to get surface capabilities");
        }

        uint32_t formatCount;
        result = physicalDevice.getSurfaceFormatsKHR(surface, &formatCount, nullptr);
        if (result != vk::Result::eSuccess) {
            log::globalLogger->error("failed to get surface formats");
        }

        if (formatCount > 0) {
            details.formats.resize(formatCount);
            result = physicalDevice.getSurfaceFormatsKHR(surface, &formatCount, details.formats.data());
            if (result != vk::Result::eSuccess) {
                log::globalLogger->error("failed to get surface formats");
            }
        }

        uint32_t presentModeCount;
        result = physicalDevice.getSurfacePresentModesKHR(surface, &presentModeCount, nullptr);
        if (result != vk::Result::eSuccess) {
            log::globalLogger->error("failed to get surface present modes");
        }

        if (presentModeCount > 0) {
            details.presentModes.resize(presentModeCount);
            result = physicalDevice.getSurfacePresentModesKHR(surface, &presentModeCount, details.presentModes.data());
            if (result != vk::Result::eSuccess) {
                log::globalLogger->error("failed to get surface present modes");
            }
        }

        return details;
    }

}
