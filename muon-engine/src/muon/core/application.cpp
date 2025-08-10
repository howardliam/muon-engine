#include "muon/core/application.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"

#include <memory>

namespace muon {

application::application(
    std::string_view name,
    const vk::Extent2D &extent,
    bool v_sync,
    window_mode mode
) : name_{name} {
    core::expect(!instance_, "application already exists");
    instance_ = this;

    dispatcher_ = std::make_unique<event::dispatcher>();
    window_ = std::make_unique<window>(name_, extent, mode, *dispatcher_);

    on_window_close_ = dispatcher_->subscribe<event::window_quit_event>([&](const auto &event) { running_ = false; });
}

application::~application() {}

void application::push_layer(layer *layer) {
    layer_stack_.push_layer(layer);
    layer->on_attach();
}

void application::run() {
    std::unique_lock<std::mutex> lock{run_mutex_};

    client::info("running {}", name_);

    while (running_) {
        window_->poll_events();

        for (auto &layer : layer_stack_) {
            layer->on_update();
        }
    }
}

auto application::name() const -> std::string_view { return name_; }
auto application::instance() -> reference { return *instance_; }

} // namespace muon
