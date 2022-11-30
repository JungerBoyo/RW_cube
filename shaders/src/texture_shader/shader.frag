#version 450 core

layout(location = 0) out vec4 out_fragment;

layout(binding = 1) uniform sampler2DArray u_tex_array;

layout(std140, binding = 0) uniform MVP {
    mat4 vp;
    mat4 m_position;
    vec3 light_pos;
    float ambient_light;
    vec3 camera_pos;
};

layout(location = 0) in vec2 in_texcoord;
layout(location = 1) flat in float in_tex_id;

void main() {
    vec4 tex_component = texture(u_tex_array, vec3(in_texcoord, in_tex_id));
    out_fragment = vec4(tex_component.xyz, 1.0);
}