#pragma once

#include "muon/core/layer.hpp"
#include "muon/core/layer_stack.hpp"
#include "muon/core/types.hpp"
#include "muon/utils/no_copy.hpp"
#include "muon/utils/no_move.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"

#include <memory>
#include <mutex>
#include <string_view>

namespace muon {

class Application : utils::NoCopy, utils::NoMove {
public:
    using Pointer = Application *;
    using ConstPointer = const Application *;
    using Reference = Application &;
    using ConstReference = const Application &;

    Application(
        std::string_view name,
        Extent2D extent,
        bool v_sync,
        WindowMode mode
    );
    virtual ~Application();

    void push_layer(Layer *layer);

    void run();

public:
    auto name() const -> std::string_view;
    static auto instance() -> Reference;

protected:
    std::string name_;

    LayerStack layer_stack_;

    std::unique_ptr<event::Dispatcher> dispatcher_{nullptr};
    event::Dispatcher::Handle on_window_close_{};

    std::unique_ptr<Window> window_{nullptr};

    std::mutex run_mutex_;
    bool running_{true};

    static inline Pointer instance_{nullptr};
};

auto create_application(size_t count, char **arguments) -> Application::Pointer;

} // namespace muon
