#pragma once

#include "muon/core/window.hpp"
#include "muon/graphics/frame_manager.hpp"

namespace muon {

    struct EngineContext {
        Window *window = nullptr;
        gfx::FrameManager *frameManager = nullptr;

        ~EngineContext() {
            delete window;
            delete frameManager;
        }
    };

}
