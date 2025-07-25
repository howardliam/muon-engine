#pragma once

#include "muon/core/layer.hpp"

#include <vector>

namespace muon {

class LayerStack {
public:
    LayerStack();
    ~LayerStack();

    auto pushLayer(Layer *layer) -> void;
    auto popLayer(Layer *layer) -> void;

    auto begin() -> std::vector<Layer *>::iterator { return m_layers.begin(); }
    auto end() -> std::vector<Layer *>::iterator { return m_layers.end(); }

private:
    std::vector<Layer *> m_layers{};
    std::vector<Layer *>::iterator m_layerInsert;
};

} // namespace muon
