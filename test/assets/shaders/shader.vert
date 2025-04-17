#version 460 core

vec2 positions[3] = vec2[](
        vec2(0.0, -0.5),
        vec2(0.5, 0.5),
        vec2(-0.5, 0.5)
    );

layout(location = 0) in vec3 position;

layout(set = 0, binding = 0) uniform Ubo {
    mat4 projection;
    mat4 view;
} ubo;

void main() {
    gl_Position = ubo.projection * ubo.view * vec4(positions[gl_VertexIndex], -5.0, 1.0);
}
