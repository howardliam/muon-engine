#include "muon/core/layer_stack.hpp"

#include <algorithm>

namespace muon {

LayerStack::LayerStack() { m_layerInsert = m_layers.begin(); }

LayerStack::~LayerStack() {
    for (Layer *layer : m_layers) {
        delete layer;
    }
}

auto LayerStack::pushLayer(Layer *layer) -> void { m_layerInsert = m_layers.emplace(m_layerInsert, layer); }

auto LayerStack::popLayer(Layer *layer) -> void {
    auto it = std::ranges::find(m_layers, layer);
    if (it != m_layers.end()) {
        m_layers.erase(it);
        m_layerInsert -= 1;
    }
}

} // namespace muon
