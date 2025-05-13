#include "muon/engine/debugui.hpp"

#include "muon/engine/window.hpp"
#include "muon/engine/device.hpp"
#include "muon/engine/swapchain.hpp"
#include "muon/engine/descriptor/pool.hpp"

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace muon::engine {

    DebugUi::DebugUi(Window &window, Device &device) : window(window), device(device) {
        createResources();
        // IMGUI_CHECKVERSION();
        // ImGui::CreateContext();
        // ImGuiIO &io = ImGui::GetIO();
        // ImGui::StyleColorsDark();
        // ImGui_ImplSDL3_InitForVulkan(window.getWindow());

        // ImGui_ImplVulkan_InitInfo initInfo{};
        // initInfo.ApiVersion = VK_API_VERSION_1_3;
        // initInfo.Instance = static_cast<VkInstance>(device.getInstance());
        // initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(device.getPhysicalDevice());
        // initInfo.Device = static_cast<VkDevice>(device.getDevice());
        // initInfo.QueueFamily = *device.getQueueFamilyIndices().graphicsFamily;
        // initInfo.Queue = static_cast<VkQueue>(device.getGraphicsQueue());
        // initInfo.PipelineCache = nullptr;

        // auto imGuiDescriptorPool = engine::DescriptorPool::Builder(device)
        //     .addPoolSize(vk::DescriptorType::eCombinedImageSampler, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE)
        //     .setMaxSets(1)
        //     .buildUniquePtr();
        // initInfo.DescriptorPool = static_cast<VkDescriptorPool>(imGuiDescriptorPool->getPool());

        // ImGui_ImplVulkanH_Window imGuiWindow{};
        // imGuiWindow.Surface = device.getSurface();
        // auto swapchainSupport = device.getSwapchainSupportDetails();
        // imGuiWindow.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(
        //     static_cast<VkPhysicalDevice>(device.getPhysicalDevice()),
        //     static_cast<VkSurfaceKHR>(device.getSurface()),
        //     reinterpret_cast<VkFormat *>(swapchainSupport.formats.data()),
        //     swapchainSupport.formats.size(),
        //     VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        // );
        // imGuiWindow.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(
        //     static_cast<VkPhysicalDevice>(device.getPhysicalDevice()),
        //     static_cast<VkSurfaceKHR>(device.getSurface()),
        //     reinterpret_cast<VkPresentModeKHR *>(swapchainSupport.presentModes.data()),
        //     swapchainSupport.presentModes.size()
        // );
        // ImGui_ImplVulkanH_CreateOrResizeWindow(
        //     static_cast<VkInstance>(device.getInstance()),
        //     static_cast<VkPhysicalDevice>(device.getPhysicalDevice()),
        //     static_cast<VkDevice>(device.getDevice()),
        //     &imGuiWindow,
        //     *device.getQueueFamilyIndices().graphicsFamily,
        //     nullptr,
        //     window.getExtent().width,
        //     window.getExtent().height,
        //     engine::constants::maxFramesInFlight
        // );
    }

    DebugUi::~DebugUi() {
        device.getDevice().destroyRenderPass(renderPass);
    }

    void DebugUi::createResources() {
        descriptorPool = engine::DescriptorPool::Builder(device)
            .addPoolSize(vk::DescriptorType::eCombinedImageSampler, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE)
            .setMaxSets(1)
            .buildUniquePtr();

        vk::AttachmentDescription2 colorAttachment{};
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.format = vk::Format::eB8G8R8A8Srgb;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
        colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference2 colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription2 subpass{};
        subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        vk::SubpassDependency2 dependency{};
        dependency.srcSubpass = vk::SubpassExternal;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.srcAccessMask = vk::AccessFlags{};
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        vk::RenderPassCreateInfo2 createInfo{};
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = &colorAttachment;
        createInfo.subpassCount = 1;
        createInfo.pSubpasses = &subpass;
        createInfo.dependencyCount = 1;
        createInfo.pDependencies = &dependency;

        auto result = device.getDevice().createRenderPass2(&createInfo, nullptr, &renderPass);
    }

}
