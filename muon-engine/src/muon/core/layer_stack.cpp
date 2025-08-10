#include "muon/core/layer_stack.hpp"

#include <algorithm>

namespace muon {

layer_stack::layer_stack() { layer_insert_ = layers_.begin(); }

layer_stack::~layer_stack() {
    for (layer *layer : layers_) {
        layer->on_detach();
        delete layer;
    }
}

void layer_stack::push_layer(layer *layer) {
    layer_insert_ = layers_.emplace(layer_insert_, layer);
}

void layer_stack::pop_layer(layer *layer) {
    auto it = std::ranges::find(layers_, layer);
    if (it != layers_.end()) {
        layer->on_detach();
        layers_.erase(it);
        layer_insert_ -= 1;
    }
}

auto layer_stack::begin() noexcept -> iterator { return layers_.begin(); }
auto layer_stack::begin() const noexcept -> const_iterator { return layers_.begin(); }

auto layer_stack::end() noexcept -> iterator { return layers_.end(); }
auto layer_stack::end() const noexcept -> const_iterator { return layers_.end(); }

auto layer_stack::rbegin() noexcept -> reverse_iterator { return layers_.rbegin(); }
auto layer_stack::rbegin() const noexcept -> const_reverse_iterator { return layers_.rbegin(); }

auto layer_stack::rend() noexcept -> reverse_iterator { return layers_.rend(); }
auto layer_stack::rend() const noexcept -> const_reverse_iterator { return layers_.rend(); }

constexpr auto layer_stack::size() const noexcept -> size_type { return layers_.size(); }

} // namespace muon
