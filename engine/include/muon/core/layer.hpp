#pragma once

namespace muon {

class Layer {
public:
    virtual ~Layer() = default;

    virtual auto OnAttach() -> void;
    virtual auto OnDetach() -> void;
    virtual auto OnUpdate() -> void;
};

} // namespace muon
