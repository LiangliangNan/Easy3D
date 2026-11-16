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

layout(location=0) flat in vec3 base;
layout(location=1) flat in vec3 end;
layout(location=2) flat in vec4 base_color;
layout(location=3) flat in vec4 tip_color;
layout(location=4) flat in vec2 base_uv;
layout(location=5) flat in vec2 tip_uv;
layout(location=6) flat in uvec2 id;
layout(location=7) flat in uint lighting;
layout(location=0) out vec4 out_color;
layout(location=1) out uvec2 out_id;

// Choose the nearest actual surface, including intersections before the near
// plane. Clipping that front surface must not reveal a back surface or add a cap.
void side_hit(float t, float axial_origin, float axial_direction, float length_axis,
              inout float nearest, inout int surface) {
    float z = axial_origin+t*axial_direction;
    float tolerance = 1e-6*length_axis;
    if (z >= -tolerance && z <= length_axis+tolerance && t < nearest) { nearest=t; surface=0; }
}
layout(location=8) flat in vec4 selection_tint;
void main() {
    vec3 segment = end-base;
    float length_axis = length(segment);
    if (length_axis <= 0) discard;
    vec3 axis = segment/length_axis;
    vec2 ndc = (gl_FragCoord.xy-material.viewport_origin.xy)/material.dimensions.xy*2-1; ndc.y=-ndc.y;
    vec4 near_point = material.inverse_projection*vec4(ndc,-1,1);
    vec4 far_point = material.inverse_projection*vec4(ndc,1,1);
    vec3 origin = near_point.xyz/near_point.w;
    vec3 direction = normalize(far_point.xyz-origin*far_point.w);
    vec3 offset = origin-base;
    float oz = dot(offset,axis), dz = dot(direction,axis);
    vec3 radial_origin = offset-oz*axis, radial_direction = direction-dz*axis;
    float radius = material.dimensions.w; // world-space radius of the cylinder/cone
    bool cone = material.light.w == 4;
    float slope = cone ? radius/length_axis : 0;
    float k = slope*slope, h = length_axis-oz;
    float a = dot(radial_direction,radial_direction)-k*dz*dz;
    float b = dot(radial_origin,radial_direction)+k*h*dz;
    float c = dot(radial_origin,radial_origin)-(cone ? k*h*h : radius*radius);
    // The barrel's edge field and the intersection below share one discriminant,
    // expressed with cross products to avoid subtracting large, nearly equal
    // quadratic terms at a cone tip or distant silhouette. It is evaluated here in
    // uniform control flow, like the cap-rim fields, because derivatives are
    // undefined inside the per-fragment cap tests.
    vec3 cross_term = cross(radial_origin,radial_direction);
    vec3 cone_term = h*radial_direction+dz*radial_origin;
    float positive = cone ? k*dot(cone_term,cone_term) : radius*radius*dot(radial_direction,radial_direction);
    float negative = dot(cross_term,cross_term);
    // Cap rims are conics in the pixel's ray direction. A plane-space field such
    // as R^2-|p-c|^2 is rational in the pixel coordinates (p runs to infinity for
    // a nearly edge-on cap), so its screen derivative is unbounded and a ramp
    // built on it aliases far from the rim. Clearing the ray's axial denominator
    // keeps the field a quadratic in the ray with the same zero set, and negative
    // inside the projected rim circle.
    float radial_squared = dot(radial_origin,radial_origin);
    float radial_along = dot(radial_origin,radial_direction);
    float base_rim = -((radial_squared+oz*oz-radius*radius)*dz*dz
                     - 2*oz*(oz*dz+radial_along)*dz + oz*oz);
    float end_axial = oz-length_axis;
    float end_rim = -((radial_squared+end_axial*end_axial-radius*radius)*dz*dz
                    - 2*end_axial*(end_axial*dz+radial_along)*dz + end_axial*end_axial);
    float coverage = 1, side_coverage = 1, base_coverage = 1, end_coverage = 1;
    if (!single_sample) {
        side_coverage = edge_coverage(positive-negative);
        base_coverage = edge_coverage(base_rim);
        end_coverage = edge_coverage(end_rim);
        coverage = side_coverage;
    }
    float nearest = 3.402823e38;
    int surface = -1; // 0 side, 1 base cap, 2 end cap
    // A ray parallel to a cone generator leaves a linear equation. A ray
    // parallel to a cylinder axis reaches only the caps.
    float scale = max(dot(radial_direction,radial_direction)+k*dz*dz,1e-20);
    if (abs(a) <= 1e-7*scale) {
        if (abs(b) > 1e-20) side_hit(-c/(2*b),oz,dz,length_axis,nearest,surface);
    } else {
        float discriminant = positive-negative;
        float tolerance = 1e-6*(positive+negative);
        if (discriminant >= -tolerance) {
            discriminant = max(discriminant,0);
            // Stable quadratic roots avoid cancellation at grazing angles.
            float q = -b-(b >= 0 ? 1 : -1)*sqrt(discriminant);
            if (q == 0) side_hit(-b/a,oz,dz,length_axis,nearest,surface);
            else {
                side_hit(q/a,oz,dz,length_axis,nearest,surface);
                side_hit(c/q,oz,dz,length_axis,nearest,surface);
            }
        }
    }
    // The barrel also covers this pixel whenever the side quadric is hit, even
    // if a nearer cap wins the depth test; the union below needs that fact.
    bool side_visible = surface == 0;
    if (abs(dz) > 1e-20) {
        // A multisampled fragment whose center just misses a rim still needs the
        // candidate so alpha-to-coverage can cover its inner samples, but only when
        // no side hit already covers the pixel; otherwise the rim ramp would cut a
        // seam across the barrel the cap plane merely grazes.
        float t = -oz/dz;
        vec3 radial = radial_origin+t*radial_direction;
        if ((dot(radial,radial) <= radius*radius ||
             (surface < 0 && !single_sample && base_coverage > 0)) && t >= 0 && t < nearest)
            { nearest=t; surface=1; }
        if (!cone) {
            t = (length_axis-oz)/dz;
            radial = radial_origin+t*radial_direction;
            if ((dot(radial,radial) <= radius*radius ||
                 (surface < 0 && !single_sample && end_coverage > 0)) && t >= 0 && t < nearest)
                { nearest=t; surface=2; }
        }
    }
    // A fragment whose center just misses the side outline can still cover
    // samples inside it; shade the tangent point so the mask has a surface.
    // The quadric is a*t^2 + 2*b*t + c, so the closest approach is -b/a.
    if (surface < 0 && !single_sample && side_coverage > 0 && a > 0) {
        float t = -b/a;
        side_hit(t,oz,dz,length_axis,nearest,surface);
    }
    if (surface < 0 || nearest < 0) discard;
    // The visible surface's own field defines the silhouette here: the side
    // quadric for the barrel, the disc radius for a cap. A cap pixel that also
    // lies on the barrel is covered by both surfaces, so the union keeps the
    // cap ramp from cutting a seam across the barrel at the rim.
    if (!single_sample) {
        coverage = surface == 0 ? side_coverage
                                : max(surface == 1 ? base_coverage : end_coverage, side_visible ? side_coverage : 0);
        if (coverage <= 0) discard;
    }
    vec3 hit = origin+nearest*direction;
    for (int i=0;i<int(material.clip_info.x);++i) {
        float distance = dot(material.clip_planes[i],vec4(hit,1));
        if (single_sample) {
            if (distance < 0) discard;
        } else
            coverage *= edge_coverage(distance);
    }
    vec4 clip = material.projection*vec4(hit,1);
    float depth = 0.5*(clip.z/clip.w+1);
    if (clip.w <= 0 || depth < 0 || depth > 1) discard;
    gl_FragDepth = depth;
    float axial = clamp(dot(hit-base,axis),0,length_axis);
    float fraction = axial/length_axis;
    out_color = mix(base_color,tip_color,fraction) * texture(image_texture,mix(base_uv,tip_uv,fraction));
    // Alpha-to-coverage reads the first color output's alpha on the multisampled pipeline.
    out_color.a *= coverage;
    out_id = id;
    if (lighting != 0) {
        vec3 normal = surface == 1 ? -axis : axis;
        if (surface == 0) {
            normal = hit-base-axial*axis+k*(length_axis-axial)*axis;
            if (dot(normal,normal) == 0) normal=axis; // cone tip
            else normal=normalize(normal);
        }
        if (material.shading.x != 0) {
            // Same model as the OpenGL cylinder/cone shaders: the drawable
            // material supplies ambient and a Phong specular from the shared
            // camera-space light. The cylinder is lit from both sides, the cone
            // only from its outside, matching the OpenGL diffuse terms.
            vec3 n = normalize(normal);
            vec3 view_dir = normalize(material.eye.xyz-hit);
            vec3 light_dir = normalize(material.material_light.xyz);
            float df = cone ? max(dot(n,light_dir),0) : abs(dot(n,light_dir));
            float sf = 0;
            if (df > 0) {
                sf = max(dot(normalize(light_dir+view_dir),n),0);
                sf = pow(sf,material.shading.w);
            }
            out_color.rgb = out_color.rgb*df + material.specular.rgb*sf + material.ambient.rgb;
        } else
            out_color.rgb *= 0.2+0.8*abs(dot(normal,normalize(material.light.xyz)));
    }
    out_color.rgb = mix(out_color.rgb,selection_tint.rgb,selection_tint.a);
}
