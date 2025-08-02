#pragma once

namespace muon {

class Layer {
public:
    virtual ~Layer() = default;

    virtual void onAttach() = 0;
    virtual void onDetach() = 0;
    virtual void onUpdate() = 0;
};

} // namespace muon
