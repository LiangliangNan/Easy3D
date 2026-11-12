#version 450
#extension GL_GOOGLE_include_directive : require
#include "material.glsl"
// Order-independent average-color accumulation, matching the OpenGL
// transparency/average_color_init shader: premultiplied color and alpha are
// summed, and the fragment count is accumulated separately.
layout(location=0) in vec4 vertex_color;
layout(location=3) flat in vec4 selection_tint;
layout(location=4) in vec3 world_position;
layout(location=5) in vec4 world_normal_out;
layout(location=0) out vec4 out_color;
layout(location=1) out vec4 out_count;
void main() {
    // Match OpenGL's average_color_init.frag, which shades drawable colors
    // directly (without sampling the scene texture or honoring one-sided lighting).
    vec4 color = vertex_color;
    if (material.shading.x != 0 && world_normal_out.w != 0) {
        vec3 normal = normalize(world_normal_out.xyz);
        vec3 view_dir = normalize(material.eye.xyz - world_position);
        vec3 light_dir = normalize(material.material_light.xyz);
        float diffuse = abs(dot(light_dir, normal));
        float specular = max(dot(normalize(light_dir + view_dir), normal), 0.0);
        specular = pow(specular, material.shading.w);
        color.rgb = color.rgb * diffuse + material.specular.rgb * specular + material.ambient.rgb;
    }
    color.rgb = mix(color.rgb, selection_tint.rgb, selection_tint.a);
    float alpha = clamp(color.a, 0.0, 1.0);
    out_color = vec4(color.rgb * alpha, alpha);
    out_count = vec4(1.0);
}
