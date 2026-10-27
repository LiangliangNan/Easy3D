#version 450
layout(set=0,binding=0) uniform sampler2D image_texture;
layout(set=1,binding=0,std140) uniform Points {
    mat4 model_view; mat4 projection; mat4 inverse_projection;
    vec4 dimensions; vec4 light;
} points;
layout(location=0) in vec4 vertex_color;
layout(location=1) in vec2 uv;
layout(location=2) flat in uvec2 id;
layout(location=3) flat in vec3 center;
layout(location=4) flat in uint lighting;
layout(location=5) flat in vec3 surfel_normal;
layout(location=0) out vec4 out_color;
layout(location=1) out uvec2 out_id;
layout(location=6) flat in vec4 selection_tint;
void main() {
    out_color = vertex_color * texture(image_texture,uv);
    out_id = id;
    gl_FragDepth = gl_FragCoord.z;
    if (points.light.w == 0) { out_color.rgb = mix(out_color.rgb,selection_tint.rgb,selection_tint.a); return; }
    vec2 ndc = gl_FragCoord.xy / points.dimensions.xy * 2 - 1;
    ndc.y = -ndc.y;
    vec4 a = points.inverse_projection * vec4(ndc,-1,1);
    vec4 b = points.inverse_projection * vec4(ndc,1,1);
    vec3 origin = a.xyz/a.w;
    vec3 direction = normalize(b.xyz - origin*b.w);
    vec3 hit, surface_normal;
    if (points.light.w == 2) {
        float denominator = dot(surfel_normal, direction);
        // Edge-on discs have no area. Avoid an unstable ray/plane division.
        if (abs(denominator) < 1e-7) discard;
        float t = dot(center-origin, surfel_normal) / denominator;
        if (t < 0) discard;
        hit = origin+t*direction;
        vec3 delta = hit-center;
        if (dot(delta,delta) > points.dimensions.w*points.dimensions.w) discard;
        surface_normal = surfel_normal;
    } else {
        vec3 oc = origin-center;
        float along = dot(oc,direction);
        // Cross product avoids cancellation for small spheres far from the camera.
        vec3 perpendicular = cross(oc,direction);
        float discriminant = points.dimensions.w*points.dimensions.w - dot(perpendicular,perpendicular);
        if (discriminant < 0) discard;
        float t = -along-sqrt(discriminant);
        if (t < 0) discard;
        hit = origin+t*direction;
        surface_normal = normalize(hit-center);
    }
    vec4 clip = points.projection*vec4(hit,1);
    float depth = 0.5*(clip.z/clip.w+1);
    if (clip.w <= 0 || depth < 0 || depth > 1) discard;
    gl_FragDepth = depth;
    if (lighting != 0)
        out_color.rgb *= 0.2+0.8*abs(dot(surface_normal,normalize(points.light.xyz)));
    out_color.rgb = mix(out_color.rgb,selection_tint.rgb,selection_tint.a);
}
