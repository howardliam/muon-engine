#include "muon/renderer/debug_ui.hpp"

#include "muon/core/window.hpp"
#include "muon/renderer/device.hpp"
#include "muon/renderer/swapchain.hpp"
#include "muon/renderer/image.hpp"
#include "muon/renderer/descriptor/pool.hpp"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

namespace muon {

    DebugUi::DebugUi(Window &window, Device &device) : window(window), device(device) {
        createResources();
        createSizedResources();
        initImGui();
    }

    DebugUi::~DebugUi() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        device.device().destroyFramebuffer(framebuffer);
        device.device().destroyPipelineCache(cache);
        device.device().destroyRenderPass(renderPass);
        device.device().destroyDescriptorPool(descriptorPool);
    }

    void DebugUi::pollEvents() {
        // ImGui_ImplGlfw_
    }

    void DebugUi::beginRendering(vk::CommandBuffer cmd) {
        auto extent = window.extent();

        vk::RenderPassBeginInfo beginInfo{};
        beginInfo.renderPass = renderPass;
        beginInfo.framebuffer = framebuffer;
        beginInfo.renderArea = vk::Rect2D{vk::Offset2D{}, extent};

        vk::ClearValue clearValue{};
        clearValue.color = vk::ClearColorValue{0.0f, 0.0f, 0.0f, 0.0f};
        beginInfo.clearValueCount = 1;
        beginInfo.pClearValues = &clearValue;

        cmd.beginRenderPass(&beginInfo, vk::SubpassContents::eInline);

        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        vk::Rect2D scissor{};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent = extent;

        cmd.setViewport(0, 1, &viewport);
        cmd.setScissor(0, 1, &scissor);

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void DebugUi::endRendering(vk::CommandBuffer cmd) {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), static_cast<VkCommandBuffer>(cmd));
        cmd.endRenderPass();
    }

    Image *DebugUi::getImage() const {
        return image.get();
    }

    void DebugUi::recreateSizedResources() {
        device.device().destroyFramebuffer(framebuffer);
        createSizedResources();
    }

    void DebugUi::createResources() {
        vk::DescriptorPoolSize poolSize{};
        poolSize.type = vk::DescriptorType::eCombinedImageSampler;
        poolSize.descriptorCount = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE;

        vk::DescriptorPoolCreateInfo dpCreateInfo{};
        dpCreateInfo.maxSets = 1;
        dpCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        dpCreateInfo.poolSizeCount = 1;
        dpCreateInfo.pPoolSizes = &poolSize;

        auto result = device.device().createDescriptorPool(&dpCreateInfo, nullptr, &descriptorPool);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create debug ui descriptor pool");
        }

        vk::AttachmentDescription2 colorAttachment{};
        colorAttachment.samples = vk::SampleCountFlagBits::e1;
        colorAttachment.format = format;
        colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout = layout;

        vk::AttachmentReference2 colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = layout;

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

        vk::RenderPassCreateInfo2 rpCreateInfo{};
        rpCreateInfo.attachmentCount = 1;
        rpCreateInfo.pAttachments = &colorAttachment;
        rpCreateInfo.subpassCount = 1;
        rpCreateInfo.pSubpasses = &subpass;
        rpCreateInfo.dependencyCount = 1;
        rpCreateInfo.pDependencies = &dependency;

        result = device.device().createRenderPass2(&rpCreateInfo, nullptr, &renderPass);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create debug ui render pass");
        }

        vk::PipelineCacheCreateInfo pcCreateInfo{};
        pcCreateInfo.flags = vk::PipelineCacheCreateFlags{};
        pcCreateInfo.initialDataSize = 0;
        pcCreateInfo.pInitialData = nullptr;

        result = device.device().createPipelineCache(&pcCreateInfo, nullptr, &cache);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create debug ui pipeline cache");
        }
    }

    void DebugUi::createSizedResources() {
        auto extent = window.extent();

        image = Image::Builder(device)
            .setExtent(extent)
            .setFormat(format)
            .setImageLayout(layout)
            .setImageUsageFlags(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage)
            .setAccessFlags(vk::AccessFlagBits2::eColorAttachmentWrite)
            .setPipelineStageFlags(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
            .buildUniquePtr();

        auto fbAttachment = image->getImageView();

        vk::FramebufferCreateInfo fbCreateInfo{};
        fbCreateInfo.renderPass = renderPass;
        fbCreateInfo.attachmentCount = 1;
        fbCreateInfo.pAttachments = &fbAttachment;
        fbCreateInfo.width = extent.width;
        fbCreateInfo.height = extent.height;
        fbCreateInfo.layers = 1;

        auto result = device.device().createFramebuffer(&fbCreateInfo, nullptr, &framebuffer);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("failed to create debug ui frame buffer");
        }
    }

    void DebugUi::initImGui() {
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForVulkan(reinterpret_cast<GLFWwindow *>(window.window()), false);

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.ApiVersion = VK_API_VERSION_1_3;
        initInfo.Instance = static_cast<VkInstance>(device.instance());
        initInfo.PhysicalDevice = static_cast<VkPhysicalDevice>(device.physicalDevice());
        initInfo.Device = static_cast<VkDevice>(device.device());
        initInfo.QueueFamily = *device.queueFamilyIndices()->graphicsFamily;
        initInfo.Queue = static_cast<VkQueue>(device.graphicsQueue());
        initInfo.PipelineCache = static_cast<VkPipelineCache>(cache);
        initInfo.DescriptorPool = static_cast<VkDescriptorPool>(descriptorPool);
        initInfo.RenderPass = static_cast<VkRenderPass>(renderPass);
        initInfo.MinImageCount = constants::maxFramesInFlight;
        initInfo.ImageCount = constants::maxFramesInFlight;
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&initInfo);

        ImGui_ImplVulkan_CreateFontsTexture();

        device.device().waitIdle();
    }
}
