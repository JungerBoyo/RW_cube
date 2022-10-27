#version 450 core

layout(location = 0) out vec4 out_fragment;

layout(location = 0) in vec3 in_color;

void main() {
    out_fragment = vec4(in_color, 1.0);
}