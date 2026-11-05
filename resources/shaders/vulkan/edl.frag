#version 450
// Eye-dome lighting from this frame's scene depth, in one full-screen pass.
// The VTK/EDL depth transform and obscurance are preserved; the two resolutions
// of the OpenGL implementation are approximated by 1- and 4-pixel taps.
layout(set=0,binding=0) uniform sampler2D scene_depth;
layout(location=0) in vec4 vertex_color; // near plane, far plane, scene size
layout(location=1) in vec2 uv;
layout(location=0) out vec4 out_color;

float ztransform(float z, float zn, float zf, float scene) {
    float Z = (z - 0.5) * 2.0;
    Z = -2.0 * zf * zn / ((zf - zn) * (Z - (zf + zn) / (zf - zn)));
    return 1.0 - (Z - zn) / scene;
}

float response(float t, float zj, float divisor) {
    return max(0.0, t - zj) / divisor;
}

float shade(float step_px, float divisor, float zn, float zf, float scene, vec2 texel) {
    float t = ztransform(texture(scene_depth, uv).r, zn, zf, scene);
    float total = 0.0;
    for (int c = 0; c < 8; ++c) {
        float a = 6.28318530718 * float(c) / 8.0;
        vec2 n = vec2(cos(a), sin(a));
        float zj = ztransform(texture(scene_depth, uv + step_px * texel * n).r, zn, zf, scene);
        total += response(t, zj, divisor);
    }
    return exp(-5.0 * 20.0 * total);
}

void main() {
    vec2 texel = 1.0 / vec2(textureSize(scene_depth, 0));
    float zn = vertex_color.x, zf = vertex_color.y, scene = vertex_color.z;
    float fine = shade(1.0, 1.0, zn, zf, scene, texel);
    float coarse = shade(4.0, 2.0, zn, zf, scene, texel);
    float lum = clamp(mix(fine, coarse, 0.3), 0.0, 1.0);
    out_color = vec4(0.0, 0.0, 0.0, 1.0 - lum);
}
