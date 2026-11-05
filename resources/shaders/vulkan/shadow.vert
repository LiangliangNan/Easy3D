#version 450
// Shadow-map depth pass: rasterize a scene triangle from the light.
// draw.mvp maps the camera clip position to shadow texture coordinates, so
// multiplying it by the camera clip position yields light texture coordinates.
layout(location=0) in vec3 position;
layout(push_constant) uniform Draw {
    mat4 mvp; vec4 tint; uint object_id; uint lighting; float normal_matrix[9];
} draw;
void main() {
    vec4 h = draw.mvp * vec4(position, 1.0);
    // y is not flipped: texture v=0 must stay at the light's -y clip side so
    // that the sampling shader can use the texture coordinates directly.
    gl_Position = vec4(2.0 * h.x - h.w, 2.0 * h.y - h.w, h.z, h.w);
}
