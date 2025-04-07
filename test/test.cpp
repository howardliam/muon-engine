#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <memory>
#include <muon/engine/window.hpp>
#include <muon/engine/pipeline.hpp>
#include <muon/engine/renderer.hpp>
#include <muon/engine/device.hpp>
#include <muon/assets/image.hpp>
#include <muon/misc/logger.hpp>
#include <muon/common/fs.hpp>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_enums.hpp>

namespace engine = muon::engine;
namespace window = engine::window;

class LoggerImpl : public muon::misc::ILogger {
public:
    LoggerImpl() : muon::misc::ILogger() {}

    void traceImpl(std::string message) override {
        spdlog::trace(message);
    }

    void debugImpl(std::string message) override {
        spdlog::debug(message);
    }

    void infoImpl(std::string message) override {
        spdlog::info(message);
    }

    void warnImpl(std::string message) override {
        spdlog::warn(message);
    }

    void errorImpl(std::string message) override {
        spdlog::error(message);
    }
};

int main() {
    auto logger = std::make_shared<LoggerImpl>();

    window::Properties props;
    props.height = 900;
    props.width = 1600;
    props.mode = window::DisplayMode::Windowed;
    props.title = "Testing";

    engine::Window window(props);
    engine::Device device(logger, window);
    engine::Renderer renderer(window, device);
    renderer.setClearColor({0.0f, 0.0f, 0.0f, 1.0f});

    std::filesystem::path vertPath("./test/assets/shaders/shader.vert.spv");
    std::filesystem::path fragPath("./test/assets/shaders/shader.frag.spv");
    engine::pipeline::ConfigInfo configInfo;
    engine::Pipeline::defaultConfigInfo(configInfo);

    configInfo.renderPass = renderer.getSwapchainRenderPass();

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    vk::PipelineLayout layout;
    auto result = device.getDevice().createPipelineLayout(&pipelineLayoutInfo, nullptr, &layout);
    if (result != vk::Result::eSuccess) {
        logger->error("broke");
    }
    configInfo.pipelineLayout = layout;

    engine::Pipeline pipeline(device, vertPath, fragPath, configInfo);

    std::filesystem::path imagePath("./muon-logo.png");
    auto imageData = muon::common::fs::readFile(imagePath);
    auto res = muon::assets::loadImagePng(imageData.value());
    window.setIcon(res.data);

    while (window.isOpen()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                window.setToClose();
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    window.setToClose();
                }
            }
            if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                window.resize(event.window.data1, event.window.data2);
            }
        }

        if (const auto commandBuffer = renderer.beginFrame()) {
            renderer.beginSwapchainRenderPass(commandBuffer);

            pipeline.bind(commandBuffer);
            commandBuffer.draw(3, 1, 0, 0);

            renderer.endSwapchainRenderPass(commandBuffer);
            renderer.endFrame();
        }
    }

    device.getDevice().waitIdle();
}
