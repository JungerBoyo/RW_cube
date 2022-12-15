#version 450 core

layout(location = 0) out vec4 out_fragment;

layout(binding = 2) uniform sampler2D u_tex;

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

layout(location = 0) in vec2 in_texcoord;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_position;

void main() {
    vec4 tex_component = texture(u_tex, in_texcoord);

    vec3 light_direction = normalize(light_pos - in_position);
    vec3 view_direction = normalize(camera_pos - in_position);
    vec3 reflected_light_direction = reflect(-light_direction, in_normal);

    float diffuse_component = max(dot(in_normal, light_direction), 0.0);
    float specular_component = pow(max(dot(view_direction, reflected_light_direction), 0.0), shininess);

    out_fragment = vec4(
        (
            ambient * ambient_light + 
            diffuse * diffuse_component + 
            specular * specular_component
        ) * tex_component.xyz, 
        alpha
    );
}
