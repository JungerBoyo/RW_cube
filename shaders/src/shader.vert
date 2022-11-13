#version 450 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_offset;

layout(std140, binding = 0) uniform MVP {
    mat4 mvp;
};

layout(location = 0) out vec3 out_color;

void main() {
    out_color = in_color;
    gl_Position = mvp * vec4(in_position + in_offset, 1.0);
}