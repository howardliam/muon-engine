#include "muon/core/layer_stack.hpp"

#include <algorithm>

namespace muon {

LayerStack::LayerStack() { layer_insert_ = layers_.begin(); }

LayerStack::~LayerStack() {
    for (Layer *layer : layers_) {
        layer->on_detach();
        delete layer;
    }
}

void LayerStack::push_layer(Layer *layer) {
    layer_insert_ = layers_.emplace(layer_insert_, layer);
}

void LayerStack::pop_layer(Layer *layer) {
    auto it = std::ranges::find(layers_, layer);
    if (it != layers_.end()) {
        layer->on_detach();
        layers_.erase(it);
        layer_insert_ -= 1;
    }
}

auto LayerStack::begin() noexcept -> Iterator { return layers_.begin(); }
auto LayerStack::begin() const noexcept -> ConstIterator { return layers_.begin(); }

auto LayerStack::end() noexcept -> Iterator { return layers_.end(); }
auto LayerStack::end() const noexcept -> ConstIterator { return layers_.end(); }

auto LayerStack::rbegin() noexcept -> ReverseIterator { return layers_.rbegin(); }
auto LayerStack::rbegin() const noexcept -> ConstReverseIterator { return layers_.rbegin(); }

auto LayerStack::rend() noexcept -> ReverseIterator { return layers_.rend(); }
auto LayerStack::rend() const noexcept -> ConstReverseIterator { return layers_.rend(); }

auto LayerStack::size() const noexcept -> SizeType { return layers_.size(); }

} // namespace muon
