#pragma once

namespace muon {

class Layer {
public:
    virtual ~Layer() = default;

    virtual auto onAttach() -> void = 0;
    virtual auto onDetach() -> void = 0;
    virtual auto onUpdate() -> void = 0;
};

} // namespace muon
