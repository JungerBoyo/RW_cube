#version 450 core

layout(location = 0) in vec3 in_position;

layout(std140, binding = 0) uniform MVP {
    mat4 vp;
    mat4 m_position;
    vec3 light_pos;
    float ambient_light;
    vec3 camera_pos;
};

void main() {
    gl_Position = vp * vec4(0.2*(in_position + vec3(-0.5, -0.5, -0.5)) + light_pos, 1.0);
}