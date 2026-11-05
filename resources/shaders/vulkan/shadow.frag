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

// Blocker search needs the raw depth, while the visibility filter must compare
// first and then interpolate: that is what a GL_LINEAR sampler2DShadow does on
// OpenGL. SEARCH_TAPS/FILTER_TAPS keep the PCSS cost near the OpenGL default
// (64/128 Poisson taps) while staying affordable in one full-screen pass.
const int SEARCH_TAPS = 32;
const int FILTER_TAPS = 64;
const float GOLDEN_ANGLE = 2.39996323;

vec2 disc(int i, int count, float angle) {
    float a = float(i) * GOLDEN_ANGLE + angle;
    float radius = sqrt((float(i) + 0.5) / float(count));
    return vec2(cos(a), sin(a)) * radius;
}

// Interleaved gradient noise rotates the sparse kernels per pixel, turning the
// static Poisson banding of a fixed pattern into low-level noise.
float pixel_angle() {
    vec2 p = gl_FragCoord.xy;
    return 6.2831853 * fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

float stored_depth(vec2 coord) {
    if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0)
        return 1.0;
    return texture(shadow_map, coord).r;
}

// Raw texel depth: the comparison filter below must blend comparison results,
// not depth values, so it bypasses the linear sampler used by the blocker search.
float texel_depth(ivec2 p) {
    ivec2 size = textureSize(shadow_map, 0);
    if (p.x < 0 || p.y < 0 || p.x >= size.x || p.y >= size.y)
        return 1.0;
    return texelFetch(shadow_map, p, 0).r;
}

// Comparison-then-bilinear filtering of a 2x2 shadow texel quad, matching an
// OpenGL sampler2DShadow tap. `z` is already biased.
float compare_filtered(vec2 coord, float z) {
    vec2 pos = coord * vec2(textureSize(shadow_map, 0)) - 0.5;
    vec2 base = floor(pos);
    vec2 f = pos - base;
    ivec2 p = ivec2(base);
    float s00 = texel_depth(p) < z ? 0.0 : 1.0;
    float s10 = texel_depth(p + ivec2(1, 0)) < z ? 0.0 : 1.0;
    float s01 = texel_depth(p + ivec2(0, 1)) < z ? 0.0 : 1.0;
    float s11 = texel_depth(p + ivec2(1, 1)) < z ? 0.0 : 1.0;
    return mix(mix(s00, s10, f.x), mix(s01, s11, f.x), f.y);
}

// Hard shadows use four filtered taps at +/-0.75 texel, the same offsets as the
// OpenGL shader, so their edges soften over the same footprint on both backends.
float hard_visibility(vec2 coord, float z) {
    vec2 offset = 0.75 / vec2(textureSize(shadow_map, 0));
    float visibility = 0.0;
    visibility += compare_filtered(coord + vec2(-offset.x, -offset.y), z);
    visibility += compare_filtered(coord + vec2(offset.x, -offset.y), z);
    visibility += compare_filtered(coord + vec2(-offset.x, offset.y), z);
    visibility += compare_filtered(coord + vec2(offset.x, offset.y), z);
    return visibility * 0.25;
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
        // Radii below one texel cannot be resolved by the shadow map and only add
        // noise, so PCSS operates on at least one texel.
        float texel = 1.0 / float(textureSize(shadow_map, 0).x);
        float angle = pixel_angle();
        float z_eye = clip_to_eye(coord.z);
        float search = softness * (z_eye - vertex_color.z) / max(z_eye, 1e-6);
        search = max(search, texel);
        float blocker_count = 0.0;
        float blocker_depth = 0.0;
        for (int i = 0; i < SEARCH_TAPS; ++i) {
            float sample_z = stored_depth(coord.xy + disc(i, SEARCH_TAPS, angle) * search);
            if (sample_z < coord.z - 0.005) {
                blocker_depth += sample_z;
                blocker_count += 1.0;
            }
        }
        if (blocker_count > 0.0) {
            float avg_blocker_eye = clip_to_eye(blocker_depth / blocker_count);
            float penumbra = softness * (z_eye - avg_blocker_eye) / max(avg_blocker_eye, 1e-6);
            float filter_radius = max(penumbra * vertex_color.z / max(z_eye, 1e-6), texel);
            float lit = 0.0;
            for (int i = 0; i < FILTER_TAPS; ++i)
                lit += compare_filtered(coord.xy + disc(i, FILTER_TAPS, angle) * filter_radius, coord.z - 0.005);
            visibility = lit / float(FILTER_TAPS);
        }
    } else {
        visibility = hard_visibility(coord.xy, coord.z - 0.005);
    }

    // Never make shadowed regions completely dark.
    visibility = (1.0 - darkness) + visibility * darkness;
    out_color = vec4(0.0, 0.0, 0.0, 1.0 - visibility);
}
