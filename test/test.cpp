
#include "muon/engine/swapchain.hpp"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <muon/engine/window.hpp>
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
    engine::Swapchain swapchain(device, window.getExtent());

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
        }
    }
}
