#version 450 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

layout(std140, binding = 0) uniform MVP {
    mat4 mvp;
};

layout(location = 0) out vec3 out_color;

void main() {
    out_color = in_color;
    gl_Position = mvp * vec4(in_position, 1.0);
}