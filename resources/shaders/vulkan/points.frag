#version 450
#extension GL_GOOGLE_include_directive : require
#include "material.glsl"
layout(set=0,binding=0) uniform sampler2D image_texture;

// The multisampled color pass turns this fragment's alpha into a sample mask
// (alpha-to-coverage is enabled on that pipeline only). The single-sample color
// and picking passes keep the exact center-sample rule instead, so their IDs and
// depth do not depend on screen-space derivatives.
layout(constant_id=0) const bool single_sample = true;

// Fraction of the pixel covered by a signed edge field: the screen-space
// derivative converts the field to pixel units, so the ramp is one pixel wide
// and a coverage above 0.5 means the sample at the pixel center is inside.
float edge_coverage(float distance) {
    float width = fwidth(distance);
    return width > 0 ? clamp(0.5+distance/width,0,1) : (distance >= 0 ? 1 : 0);
}

// Impostor points (spheres and surfels). The impostor surface depth differs from
// the proxy geometry depth, so this shader writes gl_FragDepth, which disables
// early depth testing for the impostor path only. Plain points use the separate
// plain-point shader, which keeps early depth testing.
layout(location=0) in vec4 vertex_color;
layout(location=1) in vec2 uv;
layout(location=2) flat in uvec2 id;
layout(location=3) flat in vec3 center;
layout(location=4) flat in uint lighting;
layout(location=5) flat in vec3 surfel_normal;
layout(location=6) flat in vec4 selection_tint;
layout(location=7) in vec3 world_position;
layout(location=8) in vec3 world_normal_out;
layout(location=0) out vec4 out_color;
layout(location=1) out uvec2 out_id;
void main() {
    out_color = vertex_color * texture(image_texture,uv);
    out_id = id;
    vec2 ndc = (gl_FragCoord.xy - material.viewport_origin.xy) / material.dimensions.xy * 2 - 1;
    ndc.y = -ndc.y;
    vec4 a = material.inverse_projection * vec4(ndc,-1,1);
    vec4 b = material.inverse_projection * vec4(ndc,1,1);
    vec3 origin = a.xyz/a.w;
    vec3 direction = normalize(b.xyz - origin*b.w);
    vec3 hit, surface_normal;
    float coverage = 1;
    // material.dimensions.w is the world-space sphere/surfel radius.
    if (material.light.w == 2) {
        float denominator = dot(surfel_normal, direction);
        // Edge-on discs have no area. Avoid an unstable ray/plane division.
        if (abs(denominator) < 1e-7) discard;
        float t = dot(center-origin, surfel_normal) / denominator;
        if (t < 0) discard;
        hit = origin+t*direction;
        vec3 delta = hit-center;
        float radial = material.dimensions.w*material.dimensions.w-dot(delta,delta);
        if (single_sample) {
            if (radial < 0) discard;
        } else
            coverage *= edge_coverage(radial);
        surface_normal = surfel_normal;
    } else {
        vec3 oc = origin-center;
        float along = dot(oc,direction);
        // Cross product avoids cancellation for small spheres far from the camera.
        vec3 perpendicular = cross(oc,direction);
        float discriminant = material.dimensions.w*material.dimensions.w - dot(perpendicular,perpendicular);
        if (single_sample) {
            if (discriminant < 0) discard;
        } else {
            coverage *= edge_coverage(discriminant);
            // The tangent point shades samples just inside a pixel the center misses.
            discriminant = max(discriminant,0);
        }
        float t = -along-sqrt(discriminant);
        if (t < 0) discard;
        hit = origin+t*direction;
        surface_normal = normalize(hit-center);
    }
    for (int i=0;i<int(material.clip_info.x);++i) {
        float distance = dot(material.clip_planes[i],vec4(hit,1));
        if (single_sample) {
            if (distance < 0) discard;
        } else
            coverage *= edge_coverage(distance);
    }
    if (!single_sample && coverage <= 0) discard;
    vec4 clip = material.projection*vec4(hit,1);
    float depth = 0.5*(clip.z/clip.w+1);
    if (clip.w <= 0 || depth < 0 || depth > 1) discard;
    gl_FragDepth = depth;
    if (lighting != 0) {
        if(material.shading.x!=0)
            out_color=shade_material(out_color,surface_normal,hit,
                material.light.w==2 && dot(surface_normal,-hit)<0);
        else out_color.rgb *= 0.2+0.8*abs(dot(surface_normal,normalize(material.light.xyz)));
    }
    out_color.rgb = mix(out_color.rgb,selection_tint.rgb,selection_tint.a);
    // Alpha-to-coverage reads the first color output's alpha on the multisampled pipeline.
    out_color.a *= coverage;
}
