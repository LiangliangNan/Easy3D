#version 450
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec4 color;
layout(location=3) in vec2 texcoord;
layout(location=4) in uint primitive_id;
layout(location=5) in vec3 end_position;
layout(location=6) in vec4 end_color;
layout(location=7) in vec2 end_texcoord;
layout(location=8) in vec3 end_normal;
layout(push_constant) uniform Draw {
    mat4 mvp; vec4 tint; uint object_id; uint lighting; float normal_matrix[9];
} draw;
layout(set=1,binding=0,std140) uniform Impostor {
    mat4 model_view; mat4 projection; mat4 inverse_projection;
    vec4 dimensions; // framebuffer width, height, pixel width, view-space radius
    vec4 light; // shared uniform layout; unused by ribbons
} shape;
layout(location=0) out vec4 ribbon_color;
layout(location=1) out vec2 ribbon_uv;
layout(location=2) flat out uvec2 id;
bool clip_plane(float a, float b, inout float lo, inout float hi) {
    if (a < 0 && b < 0) return false;
    if (a < 0) lo=max(lo,a/(a-b));
    if (b < 0) hi=min(hi,a/(a-b));
    return lo <= hi;
}
vec4 lit_color(vec4 color, vec3 normal) {
    mat3 n = mat3(vec3(draw.normal_matrix[0],draw.normal_matrix[1],draw.normal_matrix[2]),
                  vec3(draw.normal_matrix[3],draw.normal_matrix[4],draw.normal_matrix[5]),
                  vec3(draw.normal_matrix[6],draw.normal_matrix[7],draw.normal_matrix[8]));
    vec3 wn = n*normal;
    if (draw.lighting != 0 && dot(wn,wn) > 0)
        color.rgb *= 0.2+0.8*abs(dot(normalize(wn),normalize(vec3(0.3,0.5,1))));
    return color;
}
void main() {
    id = uvec2(draw.object_id,primitive_id);
    vec4 start_color = color*draw.tint, finish_color = end_color*draw.tint;
    ribbon_color = start_color; ribbon_uv = texcoord;
    vec4 a = draw.mvp*vec4(position,1), b = draw.mvp*vec4(end_position,1);
    float lo=0, hi=1;
    // Clip the centerline in homogeneous coordinates before perspective
    // division, preserving the original endpoint interpolation parameters.
    float eye_epsilon = 1e-7*max(max(abs(a.w),abs(b.w)),1e-20);
    if (!clip_plane(a.z+a.w,b.z+b.w,lo,hi) ||
        !clip_plane(a.w-a.z,b.w-b.z,lo,hi) ||
        !clip_plane(a.w-eye_epsilon,b.w-eye_epsilon,lo,hi)) {
        gl_Position=vec4(2,2,2,1); return;
    }
    vec4 start = mix(a,b,lo), finish = mix(a,b,hi);
    start = vec4(start.x,-start.y,0.5*(start.z+start.w),start.w);
    finish = vec4(finish.x,-finish.y,0.5*(finish.z+finish.w),finish.w);
    vec2 delta = (finish.xy/finish.w-start.xy/start.w)*shape.dimensions.xy;
    float length_pixels = length(delta);
    if (length_pixels <= 0) { gl_Position=vec4(2,2,2,1); return; }
    vec2 offset = vec2(-delta.y,delta.x)/length_pixels*shape.dimensions.z/shape.dimensions.xy;
    const int ends[6] = int[](0,1,1,0,1,0);
    const float sides[6] = float[](-1,-1,1,-1,1,1);
    bool at_end = ends[gl_VertexIndex] != 0;
    vec4 p = at_end ? finish : start;
    p.xy += sides[gl_VertexIndex]*offset*p.w;
    gl_Position = p;
    float t = at_end ? hi : lo;
    ribbon_color = mix(lit_color(start_color,normal),lit_color(finish_color,end_normal),t);
    ribbon_uv = mix(texcoord,end_texcoord,t);
}
