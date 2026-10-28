#version 450
layout(constant_id=0) const bool native_point=false;
layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec4 color;
layout(location=3) in vec2 texcoord;
layout(location=4) in uint primitive_id;
layout(location=9) in vec4 highlight_color;
layout(location=6) flat out vec4 selection_tint;
layout(push_constant) uniform Draw {
    mat4 mvp; vec4 tint; uint object_id; uint lighting; float normal_matrix[9];
} draw;
layout(set=1,binding=0,std140) uniform Points {
    mat4 model_view; mat4 projection; mat4 inverse_projection;
    vec4 dimensions; // width, height, pixel diameter, view-space radius
    vec4 light; // view-space direction; w = style (0 plain, 1 sphere, 2 surfel)
    vec4 clip_planes[2]; vec4 clip_info;
} points;
layout(location=0) out vec4 vertex_color;
layout(location=1) out vec2 uv;
layout(location=2) flat out uvec2 id;
layout(location=3) flat out vec3 center;
layout(location=4) flat out uint lighting;
layout(location=5) flat out vec3 surfel_normal;
void main() {
    selection_tint = highlight_color;
    const vec2 corners[6] = vec2[](vec2(-1,-1),vec2(1,-1),vec2(1,1),vec2(-1,-1),vec2(1,1),vec2(-1,1));
    vec2 corner=vec2(0);
    if (!native_point) corner=corners[gl_VertexIndex];
    gl_PointSize=1;
    vertex_color = color * draw.tint;
    uv = texcoord;
    id = uvec2(draw.object_id, primitive_id);
    lighting = draw.lighting;
    surfel_normal = vec3(0,0,1);
    if (points.light.w == 2)
        surfel_normal = normalize(transpose(inverse(mat3(points.model_view))) * normal);
    center = (points.model_view * vec4(position,1)).xyz;
    if (points.light.w == 0 || points.clip_info.y>0) {
        for (int i=0;i<int(points.clip_info.x);++i) if (dot(points.clip_planes[i],vec4(center,1))<0) {
            gl_Position=vec4(2,2,2,1); return;
        }
    }
    if (points.light.w == 0) {
        vec4 p = draw.mvp * vec4(position,1);
        gl_Position = vec4(p.x,-p.y,0.5*(p.z+p.w),p.w);
        gl_Position.xy += corner * points.dimensions.z / points.dimensions.xy * p.w;
        // Preserve the native point center's near/far clipping semantics.
        if (p.w <= 0 || p.z < -p.w || p.z > p.w) gl_Position = vec4(2,2,2,1);
        mat3 n = mat3(vec3(draw.normal_matrix[0],draw.normal_matrix[1],draw.normal_matrix[2]),
                      vec3(draw.normal_matrix[3],draw.normal_matrix[4],draw.normal_matrix[5]),
                      vec3(draw.normal_matrix[6],draw.normal_matrix[7],draw.normal_matrix[8]));
        vec3 wn = n * normal;
        if (draw.lighting != 0 && dot(wn,wn)>0)
            vertex_color.rgb *= 0.2+0.8*abs(dot(normalize(wn),normalize(vec3(0.3,0.5,1))));
    } else {
        // Project a bounding cube. If it crosses the eye plane, use the entire
        // viewport; this remains conservative even for near-plane intersections.
        vec2 lo = vec2(1e30), hi = vec2(-1e30);
        bool crosses_eye = false;
        for (int i=0;i<8;++i) {
            vec3 sign_corner = vec3((i&1)==0?-1:1,(i&2)==0?-1:1,(i&4)==0?-1:1);
            vec4 p = points.projection * vec4(center + sign_corner*points.dimensions.w,1);
            if (p.w <= 0) crosses_eye = true;
            else { lo=min(lo,p.xy/p.w); hi=max(hi,p.xy/p.w); }
        }
        if (crosses_eye) { lo=vec2(-1); hi=vec2(1); }
        lo=max(lo,vec2(-1)); hi=min(hi,vec2(1));
        vec2 p = mix(lo,hi,0.5*(corner+1));
        // Proxy depth must not clip the actual impostor surface.
        gl_Position = vec4(p.x,-p.y,0,1);
    }
}
