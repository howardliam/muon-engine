#version 450 core

layout(location = 1) in vec3 position;

void main() {
    gl_Position = vec4(position, 1.0);
}
