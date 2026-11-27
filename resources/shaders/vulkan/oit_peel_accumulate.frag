#version 450
// Resolve four supersampled peel texels into one premultiplied output pixel.
layout(set=0,binding=0) uniform sampler2D layer;
layout(location=0) out vec4 out_color;
void main() {
    ivec2 pixel = ivec2(gl_FragCoord.xy) * 2;
    out_color = 0.25 * (texelFetch(layer, pixel, 0) +
                        texelFetch(layer, pixel + ivec2(1, 0), 0) +
                        texelFetch(layer, pixel + ivec2(0, 1), 0) +
                        texelFetch(layer, pixel + ivec2(1, 1), 0));
}
