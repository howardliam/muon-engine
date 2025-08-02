#pragma once

#include "muon/asset/asset_manager.hpp"
#include "muon/core/layer_stack.hpp"
#include "muon/core/no_copy.hpp"
#include "muon/core/no_move.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/graphics/context.hpp"
#include "muon/graphics/renderer.hpp"

#include <memory>
#include <string_view>

namespace muon {

class Application : NoCopy, NoMove {
public:
    Application(
        const std::string_view name,
        const vk::Extent2D &extent,
        const bool vsync,
        const WindowMode mode
    );
    virtual ~Application();

    auto pushLayer(Layer *layer) -> void;

    auto run() -> void;

public:
    auto getName() -> const std::string_view;
    static auto get() -> Application &;

protected:
    std::string m_name;

    LayerStack m_layerStack;

    std::unique_ptr<event::Dispatcher> m_dispatcher{nullptr};
    event::Dispatcher::Handle m_onWindowClose{};

    std::unique_ptr<Window> m_window{nullptr};
    std::unique_ptr<graphics::Context> m_context{nullptr};
    std::unique_ptr<graphics::Renderer> m_renderer{nullptr};
    std::unique_ptr<asset::AssetManager> m_assetManager{nullptr};

    bool m_running{false};

    static inline Application *s_instance{nullptr};
};

auto createApplication(size_t argCount, char **argArray) -> Application *;

} // namespace muon
