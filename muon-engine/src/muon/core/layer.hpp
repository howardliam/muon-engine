#pragma once

namespace muon {

class layer {
public:
    virtual ~layer() = default;

    virtual void on_attach() = 0;
    virtual void on_detach() = 0;
    virtual void on_update() = 0;
};

} // namespace muon
