
#include <SDL3/SDL_events.h>
#include <muon/engine/window.hpp>

namespace window = muon::engine::window;

int main() {
    window::Window::Properties properties;
    properties.height = 900;
    properties.width = 1600;

    window::Window window{properties};

    while (window.isOpen()) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                window.setToClose();
            }
        }
    }
}
