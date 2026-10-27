#version 450
layout(location=0) in vec3 position;
layout(location=2) in vec4 color;
layout(location=3) in vec2 texcoord;
layout(location=4) in uint primitive_id;
layout(location=5) in vec3 end_position;
layout(location=6) in vec4 end_color;
layout(location=7) in vec2 end_texcoord;
layout(location=9) in vec4 highlight_color;
layout(location=8) flat out vec4 selection_tint;
layout(push_constant) uniform Draw {
    mat4 mvp; vec4 tint; uint object_id; uint lighting; float normal_matrix[9];
} draw;
layout(set=1,binding=0,std140) uniform Impostor {
    mat4 model_view; mat4 projection; mat4 inverse_projection;
    vec4 dimensions; // framebuffer width, height, unused, view-space radius
    vec4 light; // view-space direction; w = 3 cylinder, 4 cone
    vec4 clip_planes[2]; vec4 clip_info;
} shape;
layout(location=0) flat out vec3 base;
layout(location=1) flat out vec3 end;
layout(location=2) flat out vec4 base_color;
layout(location=3) flat out vec4 tip_color;
layout(location=4) flat out vec2 base_uv;
layout(location=5) flat out vec2 tip_uv;
layout(location=6) flat out uvec2 id;
layout(location=7) flat out uint lighting;
void main() {
    selection_tint = highlight_color;
    base = (shape.model_view * vec4(position,1)).xyz;
    end = (shape.model_view * vec4(end_position,1)).xyz;
    base_color = color * draw.tint; tip_color = end_color * draw.tint;
    base_uv = texcoord; tip_uv = end_texcoord;
    id = uvec2(draw.object_id,primitive_id); lighting = draw.lighting;
    // Collapsed segments have no well-defined axis or cone tip.
    if (all(equal(base,end))) { gl_Position = vec4(2,2,2,1); return; }
    vec3 lower = min(base,end)-shape.dimensions.w;
    vec3 upper = max(base,end)+shape.dimensions.w;
    vec2 lo = vec2(1e30), hi = vec2(-1e30);
    bool crosses_eye = false;
    for (int i=0;i<8;++i) {
        vec3 corner = vec3((i&1)==0?lower.x:upper.x,(i&2)==0?lower.y:upper.y,(i&4)==0?lower.z:upper.z);
        vec4 p = shape.projection * vec4(corner,1);
        if (p.w <= 0) crosses_eye = true;
        else { lo=min(lo,p.xy/p.w); hi=max(hi,p.xy/p.w); }
    }
    if (crosses_eye) { lo=vec2(-1); hi=vec2(1); }
    lo=max(lo,vec2(-1)); hi=min(hi,vec2(1));
    const vec2 corners[6] = vec2[](vec2(0,0),vec2(1,0),vec2(1,1),vec2(0,0),vec2(1,1),vec2(0,1));
    vec2 p = mix(lo,hi,corners[gl_VertexIndex]);
    gl_Position = vec4(p.x,-p.y,0,1);
}
