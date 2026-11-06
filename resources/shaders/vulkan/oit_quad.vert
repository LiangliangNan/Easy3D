#version 450
// Device-internal full-screen triangle. Vertex 0 maps to the top-left texel so
// the UVs address the accumulation targets in the same row order they were
// rendered (row zero is the top row).
layout(location=0) out vec2 uv;
void main() {
    vec2 p = vec2(float((gl_VertexIndex << 1) & 2), float(gl_VertexIndex & 2));
    uv = p;
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
