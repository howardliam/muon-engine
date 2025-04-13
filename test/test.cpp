#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <memory>
#include <muon/engine/window.hpp>
#include <muon/engine/pipeline.hpp>
#include <muon/engine/framehandler.hpp>
#include <muon/engine/device.hpp>
#include <muon/assets/image.hpp>
#include <muon/assets/model.hpp>
#include <muon/misc/logger.hpp>
#include <muon/common/fs.hpp>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>

using namespace muon;

class LoggerImpl : public misc::ILogger {
public:
    LoggerImpl() : misc::ILogger() {}

private:
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

    engine::window::Properties props;
    props.height = 900;
    props.width = 1600;
    props.mode = engine::window::DisplayMode::Windowed;
    props.title = "Testing";

    engine::Window window(props);
    engine::Device device(logger, window);
    engine::FrameHandler frameHandler(window, device);
    frameHandler.setClearColor({0.0f, 0.0f, 0.0f, 1.0f});

    std::filesystem::path vertPath("./test/assets/shaders/shader.vert.spv");
    std::filesystem::path fragPath("./test/assets/shaders/shader.frag.spv");

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

    engine::pipeline::ConfigInfo configInfo;
    engine::pipeline::defaultConfigInfo(configInfo);
    configInfo.renderPass = frameHandler.getSwapchainRenderPass();
    configInfo.pipelineLayout = layout;

    engine::Pipeline pipeline = engine::Pipeline::Builder(device)
        .addShader(vk::ShaderStageFlagBits::eVertex, vertPath)
        .addShader(vk::ShaderStageFlagBits::eFragment, fragPath)
        .build(configInfo);

    std::filesystem::path imagePath("./muon-logo.png");
    auto fileData = muon::common::fs::readFile(imagePath);
    auto image = muon::assets::loadImagePng(fileData.value());
    window.setIcon(image.data);

    /* assimp doesn't load fbx, glb/gltf + bin, usdc??? */
    // std::filesystem::path modelPath("test/assets/models/Cube.obj");
    // auto model = muon::assets::loadModel(modelPath);
    // if (model.has_value()) {
    //     if (model.value()->HasPositions()) {
    //         logger->info("number of vertices: {}", model.value()->mNumVertices);
    //         for (uint32_t i = 0; i < model.value()->mNumVertices; i++) {
    //             logger->info("x: {}, y: {}, z: {}", model.value()->mVertices[i].x, model.value()->mVertices[i].y, model.value()->mVertices[i].z);
    //         }
    //     } else {
    //         logger->error("model not loaded correctly");
    //     }
    // }

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

        if (const auto commandBuffer = frameHandler.beginFrame()) {
            frameHandler.beginSwapchainRenderPass(commandBuffer);

            pipeline.bind(commandBuffer);
            commandBuffer.draw(3, 1, 0, 0);

            frameHandler.endSwapchainRenderPass(commandBuffer);
            frameHandler.endFrame();
        }
    }

    device.getDevice().destroyPipelineLayout(configInfo.pipelineLayout, nullptr);
    device.getDevice().waitIdle();
}
