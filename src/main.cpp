#include <cstdlib>
#include <exception>
#include <SDL3/SDL_events.h>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>
#include <toml++/toml.hpp>

#include "core/logging.hpp"
#include "core/window.hpp"
#include "engine/vulkan/device.hpp"
#include "engine/vulkan/swapchain.hpp"

namespace muon {}
using namespace muon;

int main() {
    /* Configure termination function */
    std::set_terminate([] {
        spdlog::critical("Critical failure, program execution terminated");

        spdlog::default_logger()->flush();
        spdlog::get("Vulkan")->flush();
        spdlog::get("Window")->flush();

        std::abort();
    });

    /* Load log configs and create the loggers */
    toml::table logsConfig = toml::parse_file("logs.toml");
    logging::LogInfo logInfo{};
    logging::parseLogConfig(logsConfig, logInfo);

    /* Create default logger */
    spdlog::set_default_logger(logging::createLogger(logInfo.loggers["Main"]));

    /* Create and register specialised loggers */
    spdlog::register_logger(logging::createLogger(logInfo.loggers["Vulkan"]));
    spdlog::register_logger(logging::createLogger(logInfo.loggers["Window"]));

    /* Load main config */
    toml::table config = toml::parse_file("config.toml");

    /* Actual program begins */
    spdlog::info("Starting up");

    window::Window window{ spdlog::get("Window"), config };
    window.setIcon("muon/assets/muon-logo.png");

    engine::Device device{ spdlog::get("Vulkan"), window, config };
    engine::Swapchain swapchain{ spdlog::get("Vulkan"), device, window.getExtent() };

    while (window.isOpen()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                window.setToClose();
            }
        }
    }

    spdlog::info("Exiting...");
}
