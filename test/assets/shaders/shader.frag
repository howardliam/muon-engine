#version 460 core

layout(location = 0) in vec2 tex;
layout(location = 1) in vec3 position;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform Ubo {
    mat4 projection;
    mat4 view;
    mat4 transform;
} ubo;

// layout(set = 0, binding = 1) uniform sampler2D image;

void main() {
    // vec3 textureColor = texture(image, tex).rgb;
    // outColor = vec4(textureColor, 1.0);
    outColor = vec4(position, 1.0);
}
