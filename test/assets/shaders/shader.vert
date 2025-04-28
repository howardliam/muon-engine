#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex;

layout(location = 0) out vec3 color;

layout(set = 0, binding = 0) uniform Ubo {
    mat4 projection;
    mat4 view;
    mat4 transform;
} ubo;

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.transform * vec4(position, 1.0);

    color = position * normal;
}
