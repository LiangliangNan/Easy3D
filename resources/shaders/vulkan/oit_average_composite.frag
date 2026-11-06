#version 450
// Approximate average blending of the OpenGL transparency/average_color_blend
// shader, composited with premultiplied over-blending instead of replacing the
// frame: rgb = average color * (1 - T), alpha = 1 - T.
layout(set=0,binding=0) uniform sampler2D accumulated;
layout(set=0,binding=1) uniform sampler2D fragment_count;
layout(location=0) in vec2 uv;
layout(location=0) out vec4 out_color;
void main() {
    vec4 sum = texture(accumulated, uv);
    float n = texture(fragment_count, uv).r;
    if (n <= 0.0) discard;
    vec3 average = sum.a > 0.0 ? sum.rgb / sum.a : vec3(0.0);
    float average_alpha = clamp(sum.a / n, 0.0, 1.0);
    float transmittance = pow(1.0 - average_alpha, n);
    out_color = vec4(average * (1.0 - transmittance), 1.0 - transmittance);
}
