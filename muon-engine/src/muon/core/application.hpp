#pragma once

#include "muon/core/layer_stack.hpp"
#include "muon/utils/no_copy.hpp"
#include "muon/utils/no_move.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"

#include <memory>
#include <mutex>
#include <string_view>

namespace muon {

class application : utils::no_copy, utils::no_move {
public:
    using pointer = application *;
    using reference = application &;

    application(
        std::string_view name,
        const vk::Extent2D &extent,
        bool v_sync,
        window_mode mode
    );
    virtual ~application();

    void push_layer(layer *layer);

    void run();

public:
    auto name() const -> std::string_view;
    static auto instance() -> reference;

protected:
    std::string name_;

    layer_stack layer_stack_;

    std::unique_ptr<event::dispatcher> dispatcher_{nullptr};
    event::dispatcher::handle on_window_close_{};

    std::unique_ptr<window> window_{nullptr};

    std::mutex run_mutex_;
    bool running_{true};

    static inline pointer instance_{nullptr};
};

auto create_application(size_t count, char **arguments) -> application::pointer;

} // namespace muon
