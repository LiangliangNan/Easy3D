#version 450
layout(set=0,binding=0) uniform sampler2D image_texture;
layout(set=1,binding=0,std140) uniform Impostor {
    mat4 model_view; mat4 projection; mat4 inverse_projection;
    vec4 dimensions; vec4 light;
} shape;
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
    vec2 ndc = gl_FragCoord.xy/shape.dimensions.xy*2-1; ndc.y=-ndc.y;
    vec4 near_point = shape.inverse_projection*vec4(ndc,-1,1);
    vec4 far_point = shape.inverse_projection*vec4(ndc,1,1);
    vec3 origin = near_point.xyz/near_point.w;
    vec3 direction = normalize(far_point.xyz-origin*far_point.w);
    vec3 offset = origin-base;
    float oz = dot(offset,axis), dz = dot(direction,axis);
    vec3 radial_origin = offset-oz*axis, radial_direction = direction-dz*axis;
    float radius = shape.dimensions.w;
    bool cone = shape.light.w == 4;
    float slope = cone ? radius/length_axis : 0;
    float k = slope*slope, h = length_axis-oz;
    float a = dot(radial_direction,radial_direction)-k*dz*dz;
    float b = dot(radial_origin,radial_direction)+k*h*dz;
    float c = dot(radial_origin,radial_origin)-(cone ? k*h*h : radius*radius);
    float nearest = 3.402823e38;
    int surface = -1; // 0 side, 1 base cap, 2 end cap
    // A ray parallel to a cone generator leaves a linear equation. A ray
    // parallel to a cylinder axis reaches only the caps.
    float scale = max(dot(radial_direction,radial_direction)+k*dz*dz,1e-20);
    if (abs(a) <= 1e-7*scale) {
        if (abs(b) > 1e-20) side_hit(-c/(2*b),oz,dz,length_axis,nearest,surface);
    } else {
        // Equivalent discriminants expressed with cross products avoid subtracting
        // large, nearly equal quadratic terms at a cone tip or distant silhouette.
        vec3 cross_term = cross(radial_origin,radial_direction);
        vec3 cone_term = h*radial_direction+dz*radial_origin;
        float positive = cone ? k*dot(cone_term,cone_term) : radius*radius*dot(radial_direction,radial_direction);
        float negative = dot(cross_term,cross_term);
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
    if (abs(dz) > 1e-20) {
        float t = -oz/dz;
        vec3 radial = radial_origin+t*radial_direction;
        if (dot(radial,radial) <= radius*radius && t < nearest) { nearest=t; surface=1; }
        if (!cone) {
            t = (length_axis-oz)/dz;
            radial = radial_origin+t*radial_direction;
            if (dot(radial,radial) <= radius*radius && t < nearest) { nearest=t; surface=2; }
        }
    }
    if (surface < 0 || nearest < 0) discard;
    vec3 hit = origin+nearest*direction;
    vec4 clip = shape.projection*vec4(hit,1);
    float depth = 0.5*(clip.z/clip.w+1);
    if (clip.w <= 0 || depth < 0 || depth > 1) discard;
    gl_FragDepth = depth;
    float axial = clamp(dot(hit-base,axis),0,length_axis);
    float fraction = axial/length_axis;
    out_color = mix(base_color,tip_color,fraction) * texture(image_texture,mix(base_uv,tip_uv,fraction));
    out_id = id;
    if (lighting != 0) {
        vec3 normal = surface == 1 ? -axis : axis;
        if (surface == 0) {
            normal = hit-base-axial*axis+k*(length_axis-axial)*axis;
            if (dot(normal,normal) == 0) normal=axis; // cone tip
            else normal=normalize(normal);
        }
        out_color.rgb *= 0.2+0.8*abs(dot(normal,normalize(shape.light.xyz)));
    }
    out_color.rgb = mix(out_color.rgb,selection_tint.rgb,selection_tint.a);
}
