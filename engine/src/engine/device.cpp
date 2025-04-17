#include "muon/engine/device.hpp"

#include "muon/engine/window.hpp"
#include <SDL3/SDL_vulkan.h>
#include <format>
#include <print>
#include <set>
#include <stdexcept>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <vk_mem_alloc.hpp>

namespace muon::engine {

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
        misc::ILogger *logger = reinterpret_cast<misc::ILogger *>(userData);
        logger->info("{}", callbackData->pMessage);

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

    Device::Device(std::shared_ptr<misc::ILogger> logger, Window &window) : window(window), logger(logger) {
        createInstance();
        createDebugMessenger();
        createSurface();
        selectPhysicalDevice();
        createLogicalDevice();
        createAllocator();
        createCommandPool();
    }

    Device::~Device() {
        allocator.destroy();
        logger->info("destroyed allocator");

        device.destroyCommandPool(commandPool, nullptr);
        logger->info("destroyed command pool");

        device.destroy(nullptr);
        logger->info("destroyed device");

        instance.destroySurfaceKHR(surface, nullptr);
        logger->info("destroyed surface");

        destroyDebugUtilsMessenger(instance, debugMessenger, nullptr);
        logger->info("destroyed debug utils messenger");

        instance.destroy(nullptr);
        logger->info("destroyed instance");
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
            std::println("failed to allocate single time command buffer");
        }

        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

        result = commandBuffer.begin(&beginInfo);
        if (result != vk::Result::eSuccess) {
            std::println("failed to begin command buffer recording");
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
            std::println("failed to submit command buffer to graphics queue");
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

        vk::BufferImageCopy region;
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
        auto checkValidationLayerSupport = [&]() -> bool {
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

        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers were requested but are not available");
        }

        vk::ApplicationInfo appInfo;
        appInfo.pApplicationName = "Muon";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Muon";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.pNext = nullptr;

        auto getRequiredExtensions = [](bool enableValidationLayers) -> std::vector<const char *> {
            uint32_t sdlExtensionCount = 0;
            const char *const *sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

            if (sdlExtensions == nullptr) {
                throw std::runtime_error(std::format("failed to get instance extensions from SDL: {}", SDL_GetError()));
            }

            std::vector<const char *> extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);

            if (enableValidationLayers) {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }

            return extensions;
        };

        auto extensions = getRequiredExtensions(enableValidationLayers);
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        auto result = vk::createInstance(&createInfo, nullptr, &instance);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create instance");
        }

        logger->info("created instance");
    }

    void Device::createDebugMessenger() {
        if (!enableValidationLayers) {
            return;
        }

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
        createInfo.pUserData = logger.get();

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
            vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
            vk::PhysicalDeviceFeatures deviceFeatures = device.getFeatures();

            bool isDiscrete = deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
            bool hasCompleteIndices = findQueueFamilies(device).isComplete();
            bool supportsGeometryShader = deviceFeatures.geometryShader;
            bool extensionsSupported = checkDeviceExtensionSupport(device);
            bool swapchainAdequate = false;
            if (extensionsSupported) {
                SwapchainSupportDetails supportDetails = querySwapchainSupport(device);
                swapchainAdequate = !supportDetails.formats.empty() && !supportDetails.presentModes.empty();
            }

            return isDiscrete && hasCompleteIndices && supportsGeometryShader && extensionsSupported && swapchainAdequate;
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

        vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
    }

    void Device::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0;

        for (uint32_t queueFamily : uniqueQueueFamilies) {
            vk::DeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        vk::PhysicalDeviceFeatures deviceFeatures{};

        vk::DeviceCreateInfo createInfo{};
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();
        createInfo.pEnabledFeatures = &deviceFeatures;

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        auto result = physicalDevice.createDevice(&createInfo, nullptr, &device);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create a logical device");
        }

        device.getQueue(indices.graphicsFamily.value(), 0, &graphicsQueue);
        device.getQueue(indices.presentFamily.value(), 0, &presentQueue);
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

        vk::CommandPoolCreateInfo poolInfo;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;

        auto result = device.createCommandPool(&poolInfo, nullptr, &commandPool);
        if (result != vk::Result::eSuccess) {
            std::println("failed to create command pool");
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
            std::println("failed to get surface capabilities");
        }

        uint32_t formatCount;
        result = physicalDevice.getSurfaceFormatsKHR(surface, &formatCount, nullptr);
        if (result != vk::Result::eSuccess) {
            std::println("failed to get surface formats");
        }

        if (formatCount > 0) {
            details.formats.resize(formatCount);
            result = physicalDevice.getSurfaceFormatsKHR(surface, &formatCount, details.formats.data());
            if (result != vk::Result::eSuccess) {
                std::println("failed to get surface formats");
            }
        }

        uint32_t presentModeCount;
        result = physicalDevice.getSurfacePresentModesKHR(surface, &presentModeCount, nullptr);
        if (result != vk::Result::eSuccess) {
            std::println("failed to get surface present modes");
        }

        if (presentModeCount > 0) {
            details.presentModes.resize(presentModeCount);
            result = physicalDevice.getSurfacePresentModesKHR(surface, &presentModeCount, details.presentModes.data());
            if (result != vk::Result::eSuccess) {
                std::println("failed to get surface present modes");
            }
        }

        return details;
    }

}
