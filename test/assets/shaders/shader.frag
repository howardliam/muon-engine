#version 460 core

layout(location = 0) in vec3 color;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 colorReduced = color / 2.0;
    vec3 realColor = vec3(1.0, 0.45, 0.5);

    outColor = vec4(realColor - colorReduced, 1.0);
}
