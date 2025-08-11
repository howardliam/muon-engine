#include "muon/core/application.hpp"

#include "muon/core/expect.hpp"
#include "muon/core/log.hpp"
#include "muon/core/types.hpp"
#include "muon/core/window.hpp"
#include "muon/event/dispatcher.hpp"
#include "muon/event/event.hpp"

#include <memory>

namespace muon {

Application::Application(
    std::string_view name,
    Extent2D extent,
    bool v_sync,
    WindowMode mode
) : name_{name} {
    core::expect(!instance_, "application already exists");
    instance_ = this;

    dispatcher_ = std::make_unique<event::Dispatcher>();
    window_ = std::make_unique<Window>(name_, extent, mode, *dispatcher_);

    on_window_close_ = dispatcher_->subscribe<event::WindowQuit>([&](const auto &event) { running_ = false; });
}

Application::~Application() {}

void Application::push_layer(Layer *layer) {
    layer_stack_.push_layer(layer);
    layer->on_attach();
}

void Application::run() {
    std::unique_lock<std::mutex> lock{run_mutex_};

    client::info("running {}", name_);

    while (running_) {
        window_->poll_events();

        for (auto &layer : layer_stack_) {
            layer->on_update();
        }
    }
}

auto Application::name() const -> std::string_view { return name_; }
auto Application::instance() -> Reference { return *instance_; }

} // namespace muon
