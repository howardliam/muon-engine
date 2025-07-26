#pragma once

namespace muon {

class Layer {
public:
    virtual ~Layer() = default;

    virtual auto onAttach() -> void;
    virtual auto onDetach() -> void;
    virtual auto onUpdate() -> void;
};

} // namespace muon
