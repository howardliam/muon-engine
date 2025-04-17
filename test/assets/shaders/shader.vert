#version 460 core

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 color;

layout(set = 0, binding = 0) uniform Ubo {
    mat4 projection;
    mat4 view;
} ubo;

void main() {
    gl_Position = ubo.projection * ubo.view * vec4(position, 1.0);

    color = position;
}
