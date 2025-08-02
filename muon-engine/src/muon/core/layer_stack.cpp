#include "muon/core/layer_stack.hpp"

#include <algorithm>

namespace muon {

LayerStack::LayerStack() { m_layerInsert = m_layers.begin(); }

LayerStack::~LayerStack() {
    for (Layer *layer : m_layers) {
        layer->onDetach();
        delete layer;
    }
}

void LayerStack::pushLayer(Layer *layer) { m_layerInsert = m_layers.emplace(m_layerInsert, layer); }

void LayerStack::popLayer(Layer *layer) {
    auto it = std::ranges::find(m_layers, layer);
    if (it != m_layers.end()) {
        layer->onDetach();
        m_layers.erase(it);
        m_layerInsert -= 1;
    }
}

} // namespace muon
