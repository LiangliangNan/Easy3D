#version 450
#extension GL_GOOGLE_include_directive : require
#include "material.glsl"
// Full-screen shadow pass from this frame's scene depth and shadow map.
// Hard shadows compare one depth; positive softness performs a PCSS blocker
// search and a variable-size PCF filter, following the OpenGL soft shadow.
layout(set=0,binding=0) uniform sampler2D scene_depth;
layout(set=0,binding=1) uniform sampler2D shadow_map;
layout(location=0) in vec4 vertex_color; // darkness, softness, light near, light far
layout(location=1) in vec2 uv;
layout(location=0) out vec4 out_color;

const int TAPS = 16;

vec2 disc(int i) {
    float angle = float(i) * 2.39996323;
    float radius = sqrt((float(i) + 0.5) / float(TAPS));
    return vec2(cos(angle), sin(angle)) * radius;
}

float stored_depth(vec2 coord) {
    if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0)
        return 1.0;
    return texture(shadow_map, coord).r;
}

float clip_to_eye(float z) {
    float zn = vertex_color.z, zf = vertex_color.w;
    return zf * zn / (zf - z * (zf - zn));
}

void main() {
    float depth = texture(scene_depth, uv).r;
    if (depth >= 1.0) { // background
        out_color = vec4(0.0);
        return;
    }

    // Reconstruct the camera clip position, including its OpenGL depth range.
    vec4 clip = vec4(2.0 * uv.x - 1.0, 1.0 - 2.0 * uv.y, 2.0 * depth - 1.0, 1.0);
    vec4 q = material.shadow * clip;
    if (abs(q.w) < 1e-8) {
        out_color = vec4(0.0);
        return;
    }
    vec3 coord = q.xyz / q.w;
    if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0 ||
        coord.z <= 0.0 || coord.z >= 1.0) {
        out_color = vec4(0.0);
        return;
    }

    float darkness = clamp(vertex_color.x, 0.0, 1.0);
    float softness = max(vertex_color.y, 0.0);
    float visibility = 1.0;
    if (softness > 0.0) {
        float z_eye = clip_to_eye(coord.z);
        float search = softness * (z_eye - vertex_color.z) / max(z_eye, 1e-6);
        search = max(search, 0.0);
        float blocker_count = 0.0;
        float blocker_depth = 0.0;
        for (int i = 0; i < TAPS; ++i) {
            float sample_z = stored_depth(coord.xy + disc(i) * search);
            if (sample_z < coord.z - 0.005) {
                blocker_depth += sample_z;
                blocker_count += 1.0;
            }
        }
        if (blocker_count > 0.0) {
            float avg_blocker_eye = clip_to_eye(blocker_depth / blocker_count);
            float penumbra = softness * (z_eye - avg_blocker_eye) / max(avg_blocker_eye, 1e-6);
            float filter_radius = max(penumbra, 0.0) * vertex_color.z / max(z_eye, 1e-6);
            float lit = 0.0;
            for (int i = 0; i < TAPS; ++i)
                lit += coord.z - 0.005 > stored_depth(coord.xy + disc(i) * filter_radius) ? 0.0 : 1.0;
            visibility = lit / float(TAPS);
        }
    } else {
        visibility = coord.z - 0.005 > stored_depth(coord.xy) ? 0.0 : 1.0;
    }

    // Never make shadowed regions completely dark.
    visibility = (1.0 - darkness) + visibility * darkness;
    out_color = vec4(0.0, 0.0, 0.0, 1.0 - visibility);
}
