
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <muon/engine/window.hpp>
#include <muon/engine/renderer.hpp>
#include <muon/engine/device.hpp>

namespace engine = muon::engine;
namespace window = engine::window;

int main() {
    window::Properties props;
    props.height = 900;
    props.width = 1600;
    props.mode = window::DisplayMode::Windowed;
    props.title = "Testing";

    window::Window window(props);
    engine::Device device(window);
    engine::Renderer renderer(window, device);
    renderer.setClearColor({1.0f, 0.0f, 1.0f, 1.0f});

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


            renderer.endSwapchainRenderPass(commandBuffer);
            renderer.endFrame();
        }
    }

    device.getDevice().waitIdle();
}
