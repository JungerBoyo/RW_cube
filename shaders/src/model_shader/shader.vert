#version 450 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texcoord;
layout(location = 2) in vec3 in_normal;

layout(std140, binding = 0) uniform MVP {
    mat4 vp;
    mat4 m_position;
    vec3 light_pos;
    float ambient_light;
    vec3 camera_pos;
    vec3 ambient;
    float shininess;
    vec3 diffuse;
    float alpha;
    vec3 specular;
};

layout(location = 0) out vec2 out_texcoord;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_position;

void main() {
    out_texcoord = in_texcoord.xy;
    out_normal = mat3(m_position) * in_normal;
    out_position = vec3(m_position * vec4(in_position, 1.0));
    gl_Position = vp * m_position * vec4(in_position, 1.0);
}
