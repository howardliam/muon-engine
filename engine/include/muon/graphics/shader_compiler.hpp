#pragma once

#include <glslang/Include/ResourceLimits.h>

namespace muon::graphics {

class ShaderCompiler {
public:
    struct Spec {};

public:
    ShaderCompiler(const Spec &spec);
    ~ShaderCompiler();

private:
    TBuiltInResource m_resource;
};

} // namespace muon::graphics
