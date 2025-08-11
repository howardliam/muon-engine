#pragma once

namespace muon {

class Layer {
public:
    virtual ~Layer() = default;

    virtual void on_attach() = 0;
    virtual void on_detach() = 0;
    virtual void on_update() = 0;
};

} // namespace muon
