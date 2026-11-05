#version 450
#extension GL_GOOGLE_include_directive : require
#include "material.glsl"
// Depth-only screen-space ambient occlusion for this frame's scene.
// Positions and normals are reconstructed from the scene depth and the camera
// projection, so no separate geometry pass is needed. The hemisphere kernel and
// range check follow the OpenGL SSAO shaders.
layout(set=0,binding=0) uniform sampler2D scene_depth;
layout(location=0) in vec4 vertex_color; // view-space radius, bias
layout(location=1) in vec2 uv;
layout(location=0) out vec4 out_color;

const int SAMPLE_COUNT = 16;

vec3 view_position(vec2 coord) {
    float depth = texture(scene_depth, coord).r;
    vec4 p = material.inverse_projection * vec4(coord * 2.0 - 1.0, 2.0 * depth - 1.0, 1.0);
    return p.xyz / p.w;
}

// Per-pixel rotation keeps the sparse kernel from producing banding.
vec3 rotation(vec2 pixel) {
    vec3 p = vec3(pixel, pixel.x * pixel.y);
    p = fract(p * vec3(0.1031, 0.1030, 0.0973));
    p += dot(p, p.yzx + 33.33);
    return fract((p.xxy + p.yzz) * p.zyx) * 2.0 - 1.0;
}

void main() {
    float depth = texture(scene_depth, uv).r;
    if (depth >= 1.0) { // background
        out_color = vec4(0.0);
        return;
    }

    vec3 position = view_position(uv);
    vec3 normal = normalize(cross(dFdx(position), dFdy(position)));
    if (dot(normal, position) > 0.0)
        normal = -normal; // face the camera

    vec3 random = rotation(gl_FragCoord.xy);
    vec3 tangent = random - normal * dot(random, normal);
    if (dot(tangent, tangent) < 1e-8)
        tangent = cross(normal, abs(normal.y) < 0.9 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0));
    tangent = normalize(tangent);
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    float radius = vertex_color.x;
    float bias = vertex_color.y;

    float occlusion = 0.0;
    for (int i = 0; i < SAMPLE_COUNT; ++i) {
        float f = float(i);
        // Cosine-weighted golden-angle spiral, scaled towards the center.
        float phi = f * 2.39996323;
        float cos_theta = 1.0 - f / float(SAMPLE_COUNT);
        float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
        float scale = mix(0.1, 1.0, (f / float(SAMPLE_COUNT)) * (f / float(SAMPLE_COUNT)));
        vec3 probe = position + tbn * vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta) * scale * radius;

        vec4 clip = material.projection * vec4(probe, 1.0);
        if (clip.w <= 0.0)
            continue;
        vec2 coord = clip.xy / clip.w * 0.5 + 0.5;
        if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0)
            continue;
        if (texture(scene_depth, coord).r >= 1.0)
            continue;

        vec3 blocker = view_position(coord);
        float range_check = smoothstep(0.0, 1.0, radius / abs(position.z - blocker.z));
        occlusion += (blocker.z >= probe.z + bias ? 1.0 : 0.0) * range_check;
    }

    float ao = 1.0 - occlusion / float(SAMPLE_COUNT);
    out_color = vec4(0.0, 0.0, 0.0, 1.0 - ao);
}
