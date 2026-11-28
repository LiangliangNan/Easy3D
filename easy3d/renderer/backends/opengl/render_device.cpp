// SPDX-License-Identifier: GPL-3.0-or-later
#include "render_device.h"

#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/backends/opengl/overlay_renderer.h>
#include <easy3d/renderer/backends/opengl/query_state.h>
#include <easy3d/renderer/backends/clipping.h>
#include <easy3d/util/logging.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace easy3d {
    namespace graphics {
        namespace {

            const char *vertex_shader = R"(#version 150
in vec3 position;
in vec4 color;
in vec2 uv;
in vec3 normal;
in vec4 highlight;
uniform mat4 mvp, model;
uniform mat3 normal_matrix;
out vec4 tint;
out vec2 texcoord;
out vec3 world_normal, world_position;
out vec4 highlight_tint;
void main() {
    gl_Position=mvp*vec4(position,1);
    tint=color;
    texcoord=uv;
    world_normal=normal_matrix*normal;
    world_position=(model*vec4(position,1)).xyz;
    highlight_tint=highlight;
})";

            const char *fragment_shader = R"(#version 150
in vec4 tint;
in vec2 texcoord;
in vec3 world_normal, world_position;
in vec4 highlight_tint;
uniform bool lighting, material_lighting, two_sides, distinct_back_color, ignore_highlight;
uniform vec3 eye, light;
uniform vec4 ambient, specular, back_color, color;
uniform float shininess;
uniform uint object_id;
uniform bool has_primitive_ids;
uniform usamplerBuffer primitive_ids;
uniform sampler2D image;
out vec4 out_color;
out uvec2 out_id;
void main() {
    out_color=tint*color*texture(image,texcoord);
    if (lighting && dot(world_normal,world_normal)>0.0) {
        vec3 n=normalize(world_normal);
        vec3 view_dir=normalize(eye-world_position);
        if (distinct_back_color && (!gl_FrontFacing || dot(n, view_dir) < 0)) out_color.rgb=back_color.rgb;
        if (!material_lighting) {
            out_color.rgb*=0.2+0.8*abs(dot(n,normalize(vec3(0.3,0.5,1))));
        } else {
            vec3 l=normalize(light);
            float df=dot(l,n);
            df=two_sides?abs(df):max(df,0.0);
            float sf=0.0;
            if (df>0.0) {
                sf=dot(normalize(l+view_dir),n);
                sf=two_sides?abs(sf):max(sf,0.0);
                sf=pow(sf,shininess);
            }
            out_color.rgb=out_color.rgb*df+specular.rgb*sf+ambient.rgb;
        }
    }
    if (!ignore_highlight)
        out_color.rgb=mix(out_color.rgb,highlight_tint.rgb,highlight_tint.a);
    uint primitive=has_primitive_ids?texelFetch(primitive_ids,int(gl_PrimitiveID)).r:uint(gl_PrimitiveID);
    out_id=uvec2(object_id,primitive);
})";

            const char *point_vertex_shader = R"(#version 150
layout(location=0) in vec3 position;
layout(location=1) in vec4 color;
layout(location=2) in vec2 uv;
layout(location=3) in vec3 normal;
layout(location=4) in vec4 highlight;
layout(location=5) in uint primitive_id;
uniform mat4 mvp, model_view, projection, inverse_projection;
uniform vec4 tint, dimensions, viewport_origin;
uniform uint object_id;
uniform int style;
out vec4 vertex_color, selection_tint;
out vec2 texcoord;
flat out uint primitive;
flat out vec3 center, surfel_normal;
void main() {
    const vec2 corners[6]=vec2[](vec2(-1,-1),vec2(1,-1),vec2(1,1),vec2(-1,-1),vec2(1,1),vec2(-1,1));
    vec2 corner=corners[gl_VertexID];
    vertex_color=tint*color;
    texcoord=uv; primitive=primitive_id;
    selection_tint=highlight;
    center=(model_view*vec4(position,1)).xyz;
    surfel_normal=style==2?normalize(transpose(inverse(mat3(model_view)))*normal):vec3(0,0,1);
    vec3 lower=center-dimensions.w, upper=center+dimensions.w;
    if (projection[3][2]!=0.0) {
        vec4 near_point=inverse_projection*vec4(0,0,-1,1);
        float near_z=near_point.z/near_point.w;
        if (lower.z>near_z) {gl_Position=vec4(2,2,2,1);return;}
        upper.z=min(upper.z,near_z);
    }
    vec2 lo=vec2(1e30),hi=vec2(-1e30);
    for(int i=0;i<8;++i) {
        vec3 p=vec3((i&1)==0?lower.x:upper.x,(i&2)==0?lower.y:upper.y,(i&4)==0?lower.z:upper.z);
        vec4 clip=projection*vec4(p,1);
        lo=min(lo,clip.xy/clip.w);hi=max(hi,clip.xy/clip.w);
    }
    lo=max(lo,vec2(-1));hi=min(hi,vec2(1));
    gl_Position=vec4(mix(lo,hi,0.5*(corner+1)),0,1);
})";

            const char *point_fragment_shader = R"(#version 150
uniform sampler2D image;
uniform mat4 projection, inverse_projection;
uniform vec4 dimensions, viewport_origin;
uniform int style;
uniform uint object_id;
uniform bool lighting, material_lighting, single_sample;
uniform bool distinct_back_color;
uniform vec3 eye, light, sphere_light;
uniform vec2 depth_range;
uniform vec4 clip_planes[2];
uniform int clip_count;
uniform vec4 ambient, specular, back_color;
uniform float shininess;
in vec4 vertex_color, selection_tint;
in vec2 texcoord;
flat in uint primitive;
flat in vec3 center, surfel_normal;
out vec4 out_color;
out uvec2 out_id;
float edge_coverage(float d) {
    float w=fwidth(d);
    return w>0?clamp(0.5+d/w,0,1):(d>=0?1:0);
}
void main() {
    vec2 ndc=(gl_FragCoord.xy-viewport_origin.xy)/dimensions.xy*2-1;
    vec4 a=inverse_projection*vec4(ndc,-1,1),b=inverse_projection*vec4(ndc,1,1);
    vec3 origin=a.xyz/a.w,direction=normalize(b.xyz/b.w-origin);
    vec3 hit,surface_normal;
    float coverage=1;
    if(style==2) {
        float denominator=dot(surfel_normal,direction);
        if(abs(denominator)<1e-7) discard;
        float t=dot(center-origin,surfel_normal)/denominator;
        if(t<0) discard;
        hit=origin+t*direction;
        float radial=dimensions.w*dimensions.w-dot(hit-center,hit-center);
        if(single_sample) {if(radial<0) discard;} else coverage*=edge_coverage(radial);
        surface_normal=surfel_normal;
    } else {
        vec3 oc=origin-center;
        float along=dot(oc,direction);
        float discriminant=dimensions.w*dimensions.w-dot(cross(oc,direction),cross(oc,direction));
        if(single_sample) {if(discriminant<0) discard;} else {coverage*=edge_coverage(discriminant);discriminant=max(discriminant,0);}
        float t=-along-sqrt(discriminant);
        if(t<0) discard;
        hit=origin+t*direction;
        surface_normal=normalize(hit-center);
    }
    // Section 8.15: clip the actual analytic surface hit, not the proxy quad.
    for (int i = 0; i < 2; ++i) {
        if (i >= int(clip_count)) break;
        float distance = dot(clip_planes[i], vec4(hit,1));
        if (single_sample) {
            if (distance < 0) discard;
        } else {
            coverage *= edge_coverage(distance);
        }
    }
    if (!single_sample && coverage <= 0) discard;
    vec4 clip=projection*vec4(hit,1);
    float depth=0.5*(clip.z/clip.w+1);
    if(clip.w<=0 || depth<0 || depth>1) discard;
    gl_FragDepth=mix(depth_range.x,depth_range.y,depth);
    out_color=vertex_color*texture(image,texcoord);
    if(lighting) {
        if(material_lighting) {
            vec3 n=normalize(surface_normal);
            vec3 view_dir=normalize(eye-hit),light_dir=normalize(light);
            float df=style==2&&dot(n,-hit)<0?abs(dot(n,light_dir)):dot(n,light_dir);
            float sf=0;
            if(df>0) sf=pow(max(dot(normalize(light_dir+view_dir),n),0.0),shininess);
            out_color.rgb=out_color.rgb*df+specular.rgb*sf+ambient.rgb;
        } else out_color.rgb*=0.2+0.8*abs(dot(surface_normal,normalize(sphere_light)));
    }
    if (distinct_back_color && !gl_FrontFacing) out_color.rgb=back_color.rgb;
    out_color.rgb=mix(out_color.rgb,selection_tint.rgb,selection_tint.a);
    out_color.a*=coverage;
    out_id=uvec2(object_id,primitive);
})";

            const char *line_vertex_shader = R"(#version 150
layout(location=0) in vec3 position;
layout(location=1) in vec4 color;
layout(location=2) in vec2 uv;
layout(location=3) in vec3 normal;
layout(location=4) in vec4 highlight;
layout(location=5) in vec3 end_position;
layout(location=6) in vec4 end_color;
layout(location=7) in vec2 end_uv;
layout(location=8) in vec3 end_normal;
layout(location=9) in vec4 end_highlight;
layout(location=10) in uint primitive_id;
uniform mat4 mvp, model_view, projection, inverse_projection;
uniform mat3 normal_matrix;
uniform vec4 tint, dimensions;
uniform uint object_id;
uniform bool lighting, screen_space;
uniform int style;
flat out vec3 base, finish;
flat out vec4 base_color, tip_color, selection_tint;
flat out vec2 base_uv, tip_uv;
flat out uint primitive;
flat out int lighting_enabled;
flat out vec4 endpoint_clip;
out float line_fraction;
const int ends[6]=int[](0,1,1,0,1,0);
const float sides[6]=float[](-1,-1,1,-1,1,1);
bool clip_plane(float a,float b,inout float lo,inout float hi) {
    if(a<0&&b<0) return false;
    if(a<0) lo=max(lo,a/(a-b));
    if(b<0) hi=min(hi,a/(a-b));
    return lo<=hi;
}
vec4 lit(vec4 c,vec3 n) {
    vec3 wn=normal_matrix*n;
    if(lighting&&dot(wn,wn)>0) c.rgb*=0.2+0.8*abs(dot(normalize(wn),normalize(vec3(0.3,0.5,1))));
    return c;
}
void main() {
    primitive=primitive_id;lighting_enabled=lighting?1:0;endpoint_clip=vec4(0);
    base=(model_view*vec4(position,1)).xyz;finish=(model_view*vec4(end_position,1)).xyz;
    if(all(equal(base,finish))) {gl_Position=vec4(2,2,2,1);return;}
    if(style==3) {
        selection_tint=highlight;
        vec4 a=mvp*vec4(position,1),b=mvp*vec4(end_position,1);
        float lo=0,hi=1;
        float epsilon=1e-7*max(max(abs(a.w),abs(b.w)),1e-20);
        if(!clip_plane(a.z+a.w,b.z+b.w,lo,hi)||!clip_plane(a.w-a.z,b.w-b.z,lo,hi)||!clip_plane(a.w-epsilon,b.w-epsilon,lo,hi)) {gl_Position=vec4(2,2,2,1);return;}
        vec4 start=mix(a,b,lo),last=mix(a,b,hi);
        endpoint_clip=vec4(start.z,start.w,last.z,last.w);
        start=vec4(start.x,start.y,0.5*(start.z+start.w),start.w);
        last=vec4(last.x,last.y,0.5*(last.z+last.w),last.w);
        bool at_end=ends[gl_VertexID]!=0;
        if(screen_space) {
            vec2 delta=(last.xy/last.w-start.xy/start.w)*dimensions.xy;
            float length_pixels=length(delta);
            if(length_pixels<=0) {gl_Position=vec4(2,2,2,1);return;}
            vec2 offset=vec2(-delta.y,delta.x)/length_pixels*dimensions.z/dimensions.xy;
            vec4 p=at_end?last:start;p.xy+=sides[gl_VertexID]*offset*p.w;gl_Position=p;
        } else {
            vec3 axes=finish-base,view_dir=projection[2][3]!=0?normalize(-0.5*(base+finish)):vec3(0,0,1);
            vec3 offset=dimensions.w*normalize(cross(view_dir,axes));
            vec3 p=mix(base,finish,at_end?hi:lo)+sides[gl_VertexID]*offset;
            vec4 clip=projection*vec4(p,1);gl_Position=vec4(clip.x,clip.y,0.5*(clip.z+clip.w),clip.w);
        }
        float t=at_end?hi:lo;
        base_color=lit(color*tint,normal);tip_color=lit(end_color*tint,end_normal);
        base_uv=mix(uv,end_uv,t);tip_uv=base_uv;line_fraction=t;
        return;
    }
    selection_tint=highlight;base_color=color*tint;tip_color=end_color*tint;base_uv=uv;tip_uv=end_uv;line_fraction=0;
    vec3 lower=min(base,finish)-dimensions.w,upper=max(base,finish)+dimensions.w;
    if(projection[3][2]!=0.0) {
        vec4 near_point=inverse_projection*vec4(0,0,-1,1);float near_z=near_point.z/near_point.w;
        if(lower.z>near_z) {gl_Position=vec4(2,2,2,1);return;}
        upper.z=min(upper.z,near_z);
    }
    vec2 lo=vec2(1e30),hi=vec2(-1e30);
    for(int i=0;i<8;++i) {
        vec3 p=vec3((i&1)==0?lower.x:upper.x,(i&2)==0?lower.y:upper.y,(i&4)==0?lower.z:upper.z);
        vec4 clip=projection*vec4(p,1);lo=min(lo,clip.xy/clip.w);hi=max(hi,clip.xy/clip.w);
    }
    lo=max(lo,vec2(-1));hi=min(hi,vec2(1));
    const vec2 corners[6]=vec2[](vec2(0,0),vec2(1,0),vec2(1,1),vec2(0,0),vec2(1,1),vec2(0,1));
    gl_Position=vec4(mix(lo,hi,corners[gl_VertexID]),0,1);
})";

            const char *line_fragment_shader = R"(#version 150
uniform sampler2D image;
uniform mat4 projection, inverse_projection;
uniform vec4 dimensions, viewport_origin;
uniform int style;
uniform uint object_id;
uniform bool lighting, material_lighting, single_sample;
uniform vec3 eye, light;
uniform vec2 depth_range;
uniform vec4 clip_planes[2];
uniform int clip_count;
uniform vec4 ambient, specular;
uniform float shininess;
flat in vec3 base, finish;
flat in vec4 base_color, tip_color, selection_tint;
flat in vec2 base_uv, tip_uv;
flat in uint primitive;
flat in int lighting_enabled;
in float line_fraction;
out vec4 out_color;
out uvec2 out_id;
float edge_coverage(float d) {float w=fwidth(d);return w>0?clamp(0.5+d/w,0,1):(d>=0?1:0);}
void side_hit(float t,float oz,float dz,float length_axis,inout float nearest,inout int surface) {
    float z=oz+t*dz,tolerance=1e-6*length_axis;
    if(z>=-tolerance&&z<=length_axis+tolerance&&t<nearest) {nearest=t;surface=0;}
}
void main() {
    vec3 segment=finish-base;float length_axis=length(segment);
    if(length_axis<=0) discard;
    vec3 axis=segment/length_axis;
    if(style==3) {
        out_color=mix(base_color,tip_color,line_fraction)*texture(image,base_uv);
        out_color.rgb=mix(out_color.rgb,selection_tint.rgb,selection_tint.a);out_id=uvec2(object_id,primitive);return;
    }
    vec2 ndc=(gl_FragCoord.xy-viewport_origin.xy)/dimensions.xy*2-1;
    vec4 near_point=inverse_projection*vec4(ndc,-1,1),far_point=inverse_projection*vec4(ndc,1,1);
    vec3 origin=near_point.xyz/near_point.w,direction=normalize(far_point.xyz/far_point.w-origin);
    vec3 offset=origin-base;
    float oz=dot(offset,axis),dz=dot(direction,axis);
    vec3 radial_origin=offset-oz*axis,radial_direction=direction-dz*axis;
    float radius=dimensions.w;bool cone=style==5;float slope=cone?radius/length_axis:0;
    float k=slope*slope,h=length_axis-oz;
    float a=dot(radial_direction,radial_direction)-k*dz*dz;
    float b=dot(radial_origin,radial_direction)+k*h*dz;
    float c=dot(radial_origin,radial_origin)-(cone?k*h*h:radius*radius);
    vec3 cross_term=cross(radial_origin,radial_direction),cone_term=h*radial_direction+dz*radial_origin;
    float positive=cone?k*dot(cone_term,cone_term):radius*radius*dot(radial_direction,radial_direction);
    float negative=dot(cross_term,cross_term);
    float radial_squared=dot(radial_origin,radial_origin),radial_along=dot(radial_origin,radial_direction);
    float base_rim=-((radial_squared+oz*oz-radius*radius)*dz*dz-2*oz*(oz*dz+radial_along)*dz+oz*oz);
    float end_axial=oz-length_axis;
    float end_rim=-((radial_squared+end_axial*end_axial-radius*radius)*dz*dz-2*end_axial*(end_axial*dz+radial_along)*dz+end_axial*end_axial);
    float coverage=1,side_coverage=1,base_coverage=1,end_coverage=1;
    if(!single_sample) {side_coverage=edge_coverage(positive-negative);base_coverage=edge_coverage(base_rim);end_coverage=edge_coverage(end_rim);coverage=side_coverage;}
    float nearest=3.402823e38;int surface=-1;
    float scale=max(dot(radial_direction,radial_direction)+k*dz*dz,1e-20);
    if(abs(a)<=1e-7*scale) {if(abs(b)>1e-20) side_hit(-c/(2*b),oz,dz,length_axis,nearest,surface);}
    else {
        float discriminant=positive-negative,tolerance=1e-6*(positive+negative);
        if(discriminant>=-tolerance) {
            discriminant=max(discriminant,0);float q=-b-(b>=0?1:-1)*sqrt(discriminant);
            if(q==0) side_hit(-b/a,oz,dz,length_axis,nearest,surface);
            else {side_hit(q/a,oz,dz,length_axis,nearest,surface);side_hit(c/q,oz,dz,length_axis,nearest,surface);}
        }
    }
    bool side_visible=surface==0;
    if(abs(dz)>1e-20) {
        float t=-oz/dz;vec3 radial=radial_origin+t*radial_direction;
        if((dot(radial,radial)<=radius*radius||(surface<0&&!single_sample&&base_coverage>0))&&t>=0&&t<nearest) {nearest=t;surface=1;}
        if(!cone) {t=(length_axis-oz)/dz;radial=radial_origin+t*radial_direction;if((dot(radial,radial)<=radius*radius||(surface<0&&!single_sample&&end_coverage>0))&&t>=0&&t<nearest) {nearest=t;surface=2;}}
    }
    if(surface<0&&!single_sample&&side_coverage>0&&a>0) side_hit(-b/a,oz,dz,length_axis,nearest,surface);
    if(surface<0||nearest<0) discard;
    if(!single_sample) {coverage=surface==0?side_coverage:max(surface==1?base_coverage:end_coverage,side_visible?side_coverage:0);if(coverage<=0) discard;}
    vec3 hit=origin+nearest*direction;
    // Section 8.15: clip the actual analytic surface hit, not the proxy quad.
    for (int i = 0; i < 2; ++i) {
        if (i >= int(clip_count)) break;
        float distance = dot(clip_planes[i], vec4(hit,1));
        if (single_sample) { if (distance < 0) discard; }
        else coverage *= edge_coverage(distance);
    }
    vec4 clip=projection*vec4(hit,1);float depth=0.5*(clip.z/clip.w+1);
    if(clip.w<=0||depth<0||depth>1) discard;
    gl_FragDepth=mix(depth_range.x,depth_range.y,depth);
    float axial=clamp(dot(hit-base,axis),0,length_axis),fraction=axial/length_axis;
    out_color=mix(base_color,tip_color,fraction)*texture(image,mix(base_uv,tip_uv,fraction));
    out_color.a*=coverage;out_id=uvec2(object_id,primitive);
    if(lighting_enabled!=0) {
        vec3 normal=surface==1?-axis:axis;
        if(surface==0) {normal=hit-base-axial*axis+k*(length_axis-axial)*axis;normal=dot(normal,normal)==0?axis:normalize(normal);}
        if(material_lighting) {
            vec3 n=normalize(normal),view_dir=normalize(eye-hit),light_dir=normalize(light);
            float df=cone?max(dot(n,light_dir),0):abs(dot(n,light_dir)),sf=0;
            if(df>0) sf=pow(max(dot(normalize(light_dir+view_dir),n),0.0),shininess);
            out_color.rgb=out_color.rgb*df+specular.rgb*sf+ambient.rgb;
        } else out_color.rgb*=0.2+0.8*abs(dot(normal,normalize(light)));
    }
    out_color.rgb=mix(out_color.rgb,selection_tint.rgb,selection_tint.a);
})";

            const char *wide_line_fragment_shader = R"(#version 150
uniform sampler2D image;
uniform uint object_id;
uniform vec2 depth_range;
flat in vec4 base_color, tip_color, selection_tint;
flat in vec2 base_uv;
flat in uint primitive;
flat in vec4 endpoint_clip;
in float line_fraction;
out vec4 out_color;
out uvec2 out_id;
void main() {
    float clip_z=mix(endpoint_clip.x,endpoint_clip.z,line_fraction);
    float clip_w=mix(endpoint_clip.y,endpoint_clip.w,line_fraction);
    gl_FragDepth=mix(depth_range.x,depth_range.y,0.5*(clip_z/clip_w+1));
    out_color=mix(base_color,tip_color,line_fraction)*texture(image,base_uv);
    out_color.rgb=mix(out_color.rgb,selection_tint.rgb,selection_tint.a);
    out_id=uvec2(object_id,primitive);
})";

            // Device-private presentation shaders. Effects darken the resolved scene
            // with premultiplied overlays, exactly like the Vulkan compositor, so the
            // scene's IDs and depth attachment are never modified.
            const char *effect_vertex_shader = R"(#version 150
in vec3 position;
in vec2 uv;
uniform mat4 mvp;
out vec2 uv_coord;
void main() {
    gl_Position = mvp * vec4(position, 1.0);
    uv_coord = uv;
})";

            const char *texture_composite_fragment_shader = R"(#version 150
uniform sampler2D image;
in vec2 uv_coord;
out vec4 out_color;
void main() {
    out_color = texture(image, vec2(uv_coord.x, 1.0 - uv_coord.y));
})";

            // The default framebuffer's depth format is chosen by the driver and
            // need not match an FBO attachment. glBlitFramebuffer would generate
            // GL_INVALID_OPERATION on a format mismatch, so present scene depth by
            // rasterizing a fullscreen quad and writing gl_FragDepth instead.
            const char *depth_copy_fragment_shader = R"(#version 150
uniform sampler2D scene_depth;
uniform vec2 resolution;
out vec4 out_color;
void main() {
    gl_FragDepth = texture(scene_depth, gl_FragCoord.xy / resolution).r;
    out_color = vec4(0.0);
})";

            const char *shadow_depth_vertex_shader = R"(#version 150
in vec3 position;
uniform mat4 mvp;
void main() {
    vec4 h = mvp * vec4(position, 1.0);
    gl_Position = vec4(2.0 * h.x - h.w, 2.0 * h.y - h.w, 2.0 * h.z - h.w, h.w);
})";

            const char *empty_fragment_shader = R"(#version 150
out vec4 out_color;
void main() { out_color = vec4(1.0); }
)";

            const char *shadow_fragment_shader = R"(#version 150
uniform sampler2D scene_depth;
uniform sampler2D shadow_map;
uniform mat4 shadow_matrix;
uniform vec4 effect;
uniform vec2 resolution;
in vec2 uv_coord;
out vec4 out_color;
const int SEARCH_TAPS = 32;
const int FILTER_TAPS = 64;
const float GOLDEN_ANGLE = 2.39996323;
vec2 disc(int i, int count, float angle) {
    float a = float(i) * GOLDEN_ANGLE + angle;
    float radius = sqrt((float(i) + 0.5) / float(count));
    return vec2(cos(a), sin(a)) * radius;
}
float pixel_angle() {
    vec2 p = gl_FragCoord.xy;
    return 6.2831853 * fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}
float stored_depth(vec2 c) {
    if (c.x < 0.0 || c.x > 1.0 || c.y < 0.0 || c.y > 1.0)
        return 1.0;
    return texture(shadow_map, c).r;
}
float texel_depth(ivec2 p) {
    ivec2 size = textureSize(shadow_map, 0);
    if (p.x < 0 || p.y < 0 || p.x >= size.x || p.y >= size.y)
        return 1.0;
    return texelFetch(shadow_map, p, 0).r;
}
float compare_filtered(vec2 c, float z) {
    vec2 pos = c * vec2(textureSize(shadow_map, 0)) - 0.5;
    vec2 base = floor(pos), f = pos - base;
    ivec2 p = ivec2(base);
    float s00 = texel_depth(p) < z ? 0.0 : 1.0;
    float s10 = texel_depth(p + ivec2(1, 0)) < z ? 0.0 : 1.0;
    float s01 = texel_depth(p + ivec2(0, 1)) < z ? 0.0 : 1.0;
    float s11 = texel_depth(p + ivec2(1, 1)) < z ? 0.0 : 1.0;
    return mix(mix(s00, s10, f.x), mix(s01, s11, f.x), f.y);
}
float hard_visibility(vec2 c, float z) {
    vec2 o = 0.75 / vec2(textureSize(shadow_map, 0));
    return 0.25 * (compare_filtered(c + vec2(-o.x, -o.y), z) +
                   compare_filtered(c + vec2(o.x, -o.y), z) +
                   compare_filtered(c + vec2(-o.x, o.y), z) +
                   compare_filtered(c + vec2(o.x, o.y), z));
}
float clip_to_eye(float z) {
    return effect.w * effect.z / (effect.w - z * (effect.w - effect.z));
}
void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    float depth = texture(scene_depth, uv).r;
    if (depth >= 1.0) { out_color = vec4(0.0); return; }
    vec4 clip = vec4(2.0 * uv.x - 1.0, 2.0 * uv.y - 1.0, 2.0 * depth - 1.0, 1.0);
    vec4 q = shadow_matrix * clip;
    if (abs(q.w) < 1e-8) { out_color = vec4(0.0); return; }
    vec3 coord = q.xyz / q.w;
    if (coord.x < 0.0 || coord.x > 1.0 || coord.y < 0.0 || coord.y > 1.0 ||
        coord.z <= 0.0 || coord.z >= 1.0) { out_color = vec4(0.0); return; }
    float darkness = clamp(effect.x, 0.0, 1.0), softness = max(effect.y, 0.0);
    float visibility = 1.0;
    if (softness > 0.0) {
        float texel = 1.0 / float(textureSize(shadow_map, 0).x), angle = pixel_angle();
        float eye = clip_to_eye(coord.z);
        float search = max(texel, softness * (eye - effect.z) / max(eye, 1e-6));
        float blockers = 0.0, blocker_depth = 0.0;
        for (int i = 0; i < SEARCH_TAPS; ++i) {
            float sample_z = stored_depth(coord.xy + disc(i, SEARCH_TAPS, angle) * search);
            if (sample_z < coord.z - 0.005) { blocker_depth += sample_z; blockers += 1.0; }
        }
        if (blockers > 0.0) {
            float blocker_eye = clip_to_eye(blocker_depth / blockers);
            float radius = max(texel, softness * (eye - blocker_eye) / max(blocker_eye, 1e-6) *
                                    effect.z / max(eye, 1e-6));
            float lit = 0.0;
            for (int i = 0; i < FILTER_TAPS; ++i)
                lit += compare_filtered(coord.xy + disc(i, FILTER_TAPS, angle) * radius, coord.z - 0.005);
            visibility = lit / float(FILTER_TAPS);
        }
    } else
        visibility = hard_visibility(coord.xy, coord.z - 0.005);
    visibility = (1.0 - darkness) + visibility * darkness;
    out_color = vec4(0.0, 0.0, 0.0, 1.0 - visibility);
})";

            const char *ao_fragment_shader = R"(#version 150
uniform sampler2D scene_depth;
uniform mat4 projection, inverse_projection;
uniform vec4 effect;
uniform vec2 resolution;
in vec2 uv_coord;
out vec4 out_color;
const int SLICES = 12, STEPS = 12;
const float MIN_STEP_PIXELS = 1.5;
float noise(vec2 p) { return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715)))); }
vec3 view_position(vec2 uv, float depth) {
    vec4 p = inverse_projection * vec4(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    return p.xyz / p.w;
}
vec3 neighbor(vec2 c, vec3 center, out bool valid) {
    float depth = texture(scene_depth, c).r;
    valid = depth < 1.0;
    return valid ? view_position(c, depth) : center;
}
vec3 surface_normal(vec2 uv, vec3 center, vec2 texel) {
    bool vl, vr, vu, vd;
    vec3 l = neighbor(uv - vec2(texel.x, 0.0), center, vl);
    vec3 r = neighbor(uv + vec2(texel.x, 0.0), center, vr);
    vec3 u = neighbor(uv + vec2(0.0, texel.y), center, vu);
    vec3 d = neighbor(uv - vec2(0.0, texel.y), center, vd);
    const float far_away = 1e30;
    float dl = vl ? abs(l.z - center.z) : far_away, dr = vr ? abs(r.z - center.z) : far_away;
    float du = vu ? abs(u.z - center.z) : far_away, dd = vd ? abs(d.z - center.z) : far_away;
    vec3 n = cross(dl < dr ? center - l : r - center, du < dd ? u - center : center - d);
    if (dot(n, n) < 1e-20) n = -normalize(center);
    n = normalize(n);
    return dot(n, center) > 0.0 ? -n : n;
}
float horizon(vec2 c, vec3 center, vec3 normal, float radius, float bias) {
    if (c.x < 0.0 || c.x > 1.0 || c.y < 0.0 || c.y > 1.0)
        return 0.0;
    float depth = texture(scene_depth, c).r;
    if (depth >= 1.0)
        return 0.0;
    vec3 delta = view_position(c, depth) - center;
    float d2 = dot(delta, delta);
    if (d2 >= radius * radius || d2 < 1e-10)
        return 0.0;
    float elevation = dot(delta, normal);
    return elevation <= bias ? 0.0 : clamp(elevation / sqrt(d2), 0.0, 1.0);
}
void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    float depth = texture(scene_depth, uv).r;
    if (depth >= 1.0) { out_color = vec4(1.0); return; }
    vec2 size = vec2(textureSize(scene_depth, 0)), texel = 1.0 / size;
    vec3 center = view_position(uv, depth), normal = surface_normal(uv, center, texel);
    vec3 tangent = cross(normal, abs(normal.y) < 0.9 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0));
    if (dot(tangent, tangent) < 1e-12) tangent = cross(normal, vec3(0.0, 0.0, 1.0));
    tangent = normalize(tangent);
    vec3 bitangent = cross(normal, tangent);
    float radius = effect.x, bias = effect.y, jitter = noise(gl_FragCoord.xy);
    vec4 center_clip = projection * vec4(center, 1.0);
    if (center_clip.w <= 0.0) { out_color = vec4(1.0); return; }
    vec2 center_uv = vec2(center_clip.x / center_clip.w * 0.5 + 0.5,
                          center_clip.y / center_clip.w * 0.5 + 0.5);
    float visibility = 0.0;
    for (int slice = 0; slice < SLICES; ++slice) {
        float phi = (float(slice) + jitter) * 6.28318530718 / float(SLICES);
        vec3 omega = tangent * cos(phi) + bitangent * sin(phi);
        vec4 edge_clip = projection * vec4(center + omega * radius, 1.0);
        if (edge_clip.w <= 0.0) continue;
        vec2 edge_uv = vec2(edge_clip.x / edge_clip.w * 0.5 + 0.5,
                            edge_clip.y / edge_clip.w * 0.5 + 0.5);
        vec2 span = (edge_uv - center_uv) * size;
        float span_length = length(span);
        if (span_length < 1.0) continue;
        vec2 step_direction = span / span_length / size;
        float step_stride = max(MIN_STEP_PIXELS, span_length / float(STEPS));
        float hp = 0.0, hn = 0.0;
        for (int step = 0; step < STEPS; ++step) {
            float step_offset = (float(step) + 0.5) * step_stride;
            if (step_offset > span_length) break;
            hp = max(hp, horizon(uv + step_direction * step_offset, center, normal, radius, bias));
            hn = max(hn, horizon(uv - step_direction * step_offset, center, normal, radius, bias));
        }
        visibility += 1.0 - 0.5 * (hp * hp + hn * hn);
    }
    out_color = vec4(clamp(visibility / float(SLICES), 0.0, 1.0));
})";

            const char *ao_blur_fragment_shader = R"(#version 150
uniform sampler2D scene_depth, ao_raw;
uniform mat4 inverse_projection;
uniform vec4 effect;
uniform vec2 resolution;
in vec2 uv_coord;
out vec4 out_color;
const int TAPS = 4;
const float SPATIAL_SIGMA = 2.0;
vec3 view_position(vec2 uv, float depth) {
    vec4 p = inverse_projection * vec4(uv.x * 2.0 - 1.0, uv.y * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    return p.xyz / p.w;
}
vec3 neighbor(vec2 c, vec3 center, out bool valid) {
    float depth = texture(scene_depth, c).r;
    valid = depth < 1.0;
    return valid ? view_position(c, depth) : center;
}
vec3 surface_normal(vec2 uv, vec3 center, vec2 texel) {
    bool vl, vr, vu, vd;
    vec3 l = neighbor(uv - vec2(texel.x, 0.0), center, vl);
    vec3 r = neighbor(uv + vec2(texel.x, 0.0), center, vr);
    vec3 u = neighbor(uv + vec2(0.0, texel.y), center, vu);
    vec3 d = neighbor(uv - vec2(0.0, texel.y), center, vd);
    const float far_away = 1e30;
    float dl = vl ? abs(l.z - center.z) : far_away, dr = vr ? abs(r.z - center.z) : far_away;
    float du = vu ? abs(u.z - center.z) : far_away, dd = vd ? abs(d.z - center.z) : far_away;
    vec3 n = cross(dl < dr ? center - l : r - center, du < dd ? u - center : center - d);
    if (dot(n, n) < 1e-20) n = -normalize(center);
    n = normalize(n);
    return dot(n, center) > 0.0 ? -n : n;
}
void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    float depth = texture(scene_depth, uv).r;
    if (depth >= 1.0) { out_color = vec4(1.0); return; }
    vec2 texel = 1.0 / vec2(textureSize(scene_depth, 0));
    vec3 center = view_position(uv, depth), normal = surface_normal(uv, center, texel);
    float tolerance = max(0.05 * effect.x, 1e-6), sum = 0.0, weights = 0.0;
    for (int y = 0; y < TAPS; ++y) {
        for (int x = 0; x < TAPS; ++x) {
            vec2 offset = (vec2(float(x), float(y)) + 0.5 - 0.5 * float(TAPS)) * 1.5;
            vec2 coord = uv + offset * texel;
            float nd = texture(scene_depth, coord).r;
            if (nd >= 1.0) continue;
            vec3 p = view_position(coord, nd);
            float w = exp(-dot(offset, offset) / (SPATIAL_SIGMA * SPATIAL_SIGMA)) *
                      exp(-abs(dot(p - center, normal)) / tolerance);
            sum += texture(ao_raw, coord).r * w;
            weights += w;
        }
    }
    float ao = weights > 0.0 ? sum / weights : 1.0;
    out_color = vec4(clamp(mix(1.0, ao, effect.z), 0.0, 1.0));
})";

            const char *darkness_composite_fragment_shader = R"(#version 150
uniform sampler2D factor;
uniform vec2 resolution;
in vec2 uv_coord;
out vec4 out_color;
void main() {
    float value = clamp(texture(factor, gl_FragCoord.xy / resolution).r, 0.0, 1.0);
    out_color = vec4(0.0, 0.0, 0.0, 1.0 - value);
})";

            const char *edl_fragment_shader = R"(#version 150
uniform sampler2D scene_depth;
uniform vec4 effect;
uniform vec2 resolution;
in vec2 uv_coord;
out vec4 out_color;
float transform(float z, float zn, float zf, float scene) {
    float ndc = (z - 0.5) * 2.0;
    float eye = -2.0 * zf * zn / ((zf - zn) * (ndc - (zf + zn) / (zf - zn)));
    return 1.0 - (eye - zn) / scene;
}
float response(float t, float zj, float divisor) { return max(0.0, t - zj) / divisor; }
float shade(vec2 uv, float step_px, float divisor, float zn, float zf, float scene, vec2 texel) {
    float t = transform(texture(scene_depth, uv).r, zn, zf, scene), total = 0.0;
    for (int c = 0; c < 8; ++c) {
        float a = 6.28318530718 * float(c) / 8.0;
        vec2 n = vec2(cos(a), sin(a));
        total += response(t, transform(texture(scene_depth, uv + step_px * texel * n).r, zn, zf, scene), divisor);
    }
    return exp(-5.0 * 20.0 * total);
}
void main() {
    vec2 uv = gl_FragCoord.xy / resolution;
    vec2 texel = 1.0 / vec2(textureSize(scene_depth, 0));
    float lum = clamp(mix(shade(uv, 1.0, 1.0, effect.x, effect.y, effect.z, texel),
                          shade(uv, 4.0, 2.0, effect.x, effect.y, effect.z, texel), 0.3), 0.0, 1.0);
    out_color = vec4(0.0, 0.0, 0.0, 1.0 - lum);
})";

            const char *oit_fragment_shader = R"(#version 150
in vec4 tint;
in vec2 texcoord;
in vec3 world_normal, world_position;
in vec4 highlight_tint;
uniform bool lighting, material_lighting, two_sides, distinct_back_color, ignore_highlight;
uniform vec3 eye, light;
uniform vec4 ambient, specular, back_color, color;
uniform float shininess;
uniform sampler2D image;
out vec4 out_color;
out vec4 out_count;
vec4 shade_fragment() {
    vec4 result = tint * color * texture(image, texcoord);
    if (lighting && dot(world_normal, world_normal) > 0.0) {
        vec3 n = normalize(world_normal);
        vec3 view_dir = normalize(eye - world_position);
        if (distinct_back_color && (!gl_FrontFacing || dot(n, view_dir) < 0)) result.rgb = back_color.rgb;
        if (!material_lighting)
            result.rgb *= 0.2 + 0.8 * abs(dot(n, normalize(vec3(0.3, 0.5, 1))));
        else {
            vec3 l = normalize(light);
            float df = dot(l, n);
            df = two_sides ? abs(df) : max(df, 0.0);
            float sf = 0.0;
            if (df > 0.0) {
                sf = dot(normalize(l + view_dir), n);
                sf = two_sides ? abs(sf) : max(sf, 0.0);
                sf = pow(sf, shininess);
            }
            result.rgb = result.rgb * df + specular.rgb * sf + ambient.rgb;
        }
    }
    if (!ignore_highlight)
        result.rgb = mix(result.rgb, highlight_tint.rgb, highlight_tint.a);
    return result;
}
void main() {
    vec4 c = shade_fragment();
    float alpha = clamp(c.a, 0.0, 1.0);
    out_color = vec4(c.rgb * alpha, alpha);
    out_count = vec4(1.0);
})";

            const char *oit_peel_fragment_shader = R"(#version 150
in vec4 tint;
in vec2 texcoord;
in vec3 world_normal, world_position;
in vec4 highlight_tint;
uniform bool lighting, material_lighting, two_sides, distinct_back_color, ignore_highlight;
uniform vec3 eye, light;
uniform vec4 ambient, specular, back_color, color;
uniform float shininess;
uniform sampler2D image, previous_depth;
out vec4 out_color;
vec4 shade_fragment() {
    vec4 result = tint * color * texture(image, texcoord);
    if (lighting && dot(world_normal, world_normal) > 0.0) {
        vec3 n = normalize(world_normal);
        vec3 view_dir = normalize(eye - world_position);
        if (distinct_back_color && (!gl_FrontFacing || dot(n, view_dir) < 0)) result.rgb = back_color.rgb;
        if (!material_lighting)
            result.rgb *= 0.2 + 0.8 * abs(dot(n, normalize(vec3(0.3, 0.5, 1))));
        else {
            vec3 l = normalize(light);
            float df = dot(l, n);
            df = two_sides ? abs(df) : max(df, 0.0);
            float sf = 0.0;
            if (df > 0.0) {
                sf = dot(normalize(l + view_dir), n);
                sf = two_sides ? abs(sf) : max(sf, 0.0);
                sf = pow(sf, shininess);
            }
            result.rgb = result.rgb * df + specular.rgb * sf + ambient.rgb;
        }
    }
    if (!ignore_highlight)
        result.rgb = mix(result.rgb, highlight_tint.rgb, highlight_tint.a);
    return result;
}
void main() {
    vec2 previous_uv = gl_FragCoord.xy / vec2(textureSize(previous_depth, 0));
    if (gl_FragCoord.z <= texture(previous_depth, previous_uv).r)
        discard;
    vec4 c = shade_fragment();
    float alpha = clamp(c.a, 0.0, 1.0);
    if (alpha == 0.0)
        discard;
    gl_FragDepth = gl_FragCoord.z;
    out_color = vec4(c.rgb * alpha, alpha);
})";

            const char *oit_composite_fragment_shader = R"(#version 150
uniform sampler2D accumulated, fragment_count;
uniform bool use_count, supersample;
in vec2 uv_coord;
out vec4 out_color;
void main() {
    vec2 uv = vec2(uv_coord.x, 1.0 - uv_coord.y);
    vec4 result = vec4(0.0);
    if (supersample) {
        // Box-resolve premultiplied samples before placing this layer underneath accumulation.
        ivec2 pixel = ivec2(gl_FragCoord.xy) * 2;
        result = 0.25 * (texelFetch(accumulated, pixel, 0) +
                         texelFetch(accumulated, pixel + ivec2(1, 0), 0) +
                         texelFetch(accumulated, pixel + ivec2(0, 1), 0) +
                         texelFetch(accumulated, pixel + ivec2(1, 1), 0));
    } else {
        result = texture(accumulated, uv);
    }
    if (use_count) {
        float n = texture(fragment_count, uv).r;
        if (n <= 0.0) { out_color = vec4(0.0); return; }
        vec3 average = result.a > 0.0 ? result.rgb / result.a : vec3(0.0);
        float alpha = clamp(result.a / n, 0.0, 1.0);
        float transmittance = pow(max(0.0, 1.0 - alpha), n);
        result = vec4(average * (1.0 - transmittance), 1.0 - transmittance);
    }
    out_color = result;
})";

            const char *linked_vertex_shader = R"(#version 430
in vec3 position;
in vec4 color;
in vec2 uv;
in vec3 normal;
in vec4 highlight;
uniform mat4 mvp, model;
uniform mat3 normal_matrix;
out vec4 tint;
out vec2 texcoord;
out vec3 world_normal, world_position;
out vec4 highlight_tint;
void main() {
    gl_Position = mvp * vec4(position, 1.0);
    tint = color;
    texcoord = uv;
    world_normal = normal_matrix * normal;
    world_position = (model * vec4(position, 1.0)).xyz;
    highlight_tint = highlight;
})";

            const char *linked_quad_vertex_shader = R"(#version 430
in vec3 position;
in vec2 uv;
uniform mat4 mvp;
out vec2 uv_coord;
void main() {
    gl_Position = mvp * vec4(position, 1.0);
    uv_coord = uv;
})";

            const char *linked_build_fragment_shader = R"(#version 430
struct FragmentNode { vec4 color; float depth; int next; float pad0; float pad1; };
layout(std430, binding = 0) buffer HeadBuffer { int head[]; };
layout(std430, binding = 1) buffer NodeBuffer { FragmentNode nodes[]; };
layout(binding = 0) uniform atomic_uint node_counter;
uniform uint max_nodes, viewport_width;
in vec4 tint;
in vec2 texcoord;
in vec3 world_normal, world_position;
in vec4 highlight_tint;
uniform bool lighting, material_lighting, two_sides, distinct_back_color, ignore_highlight;
uniform vec3 eye, light;
uniform vec4 ambient, specular, back_color, color;
uniform float shininess;
uniform sampler2D image;
out vec4 out_color;
vec4 shade_fragment() {
    vec4 result = tint * color * texture(image, texcoord);
    if (lighting && dot(world_normal, world_normal) > 0.0) {
        vec3 n = normalize(world_normal);
        vec3 view_dir = normalize(eye - world_position);
        if (distinct_back_color && (!gl_FrontFacing || dot(n, view_dir) < 0)) result.rgb = back_color.rgb;
        if (!material_lighting)
            result.rgb *= 0.2 + 0.8 * abs(dot(n, normalize(vec3(0.3, 0.5, 1))));
        else {
            vec3 l = normalize(light);
            float df = dot(l, n);
            df = two_sides ? abs(df) : max(df, 0.0);
            float sf = 0.0;
            if (df > 0.0) {
                sf = dot(normalize(l + view_dir), n);
                sf = two_sides ? abs(sf) : max(sf, 0.0);
                sf = pow(sf, shininess);
            }
            result.rgb = result.rgb * df + specular.rgb * sf + ambient.rgb;
        }
    }
    if (!ignore_highlight)
        result.rgb = mix(result.rgb, highlight_tint.rgb, highlight_tint.a);
    return result;
}
void main() {
    out_color = vec4(0.0);
    uint index = atomicCounterIncrement(node_counter);
    if (index >= max_nodes)
        return;
    vec4 c = shade_fragment();
    float alpha = clamp(c.a, 0.0, 1.0);
    uint pixel = uint(gl_FragCoord.y) * viewport_width + uint(gl_FragCoord.x);
    nodes[index].color = vec4(c.rgb * alpha, alpha);
    nodes[index].depth = gl_FragCoord.z;
    int old = head[pixel];
    for (;;) {
        nodes[index].next = old;
        int previous = atomicCompSwap(head[pixel], old, int(index));
        if (previous == old) break;
        old = previous;
    }
})";

            const char *linked_resolve_fragment_shader = R"(#version 430
#define MAX_FRAGMENTS 64
struct FragmentNode { vec4 color; float depth; int next; float pad0; float pad1; };
layout(std430, binding = 0) buffer HeadBuffer { int head[]; };
layout(std430, binding = 1) buffer NodeBuffer { FragmentNode nodes[]; };
uniform uint viewport_width;
in vec2 uv_coord;
out vec4 out_color;
void main() {
    uint pixel = uint(gl_FragCoord.y) * viewport_width + uint(gl_FragCoord.x);
    vec4 colors[MAX_FRAGMENTS];
    float depths[MAX_FRAGMENTS];
    int count = 0;
    for (int i = head[pixel]; i != -1 && count < MAX_FRAGMENTS; i = nodes[i].next) {
        colors[count] = nodes[i].color;
        depths[count] = nodes[i].depth;
        ++count;
    }
    if (count == 0) { out_color = vec4(0.0); return; }
    for (int i = 1; i < count; ++i) {
        vec4 c = colors[i];
        float d = depths[i];
        int j = i - 1;
        while (j >= 0 && depths[j] > d) {
            colors[j + 1] = colors[j];
            depths[j + 1] = depths[j];
            --j;
        }
        colors[j + 1] = c;
        depths[j + 1] = d;
    }
    vec4 accum = vec4(0.0);
    for (int i = 0; i < count; ++i) {
        accum.rgb += colors[i].rgb * (1.0 - accum.a);
        accum.a += colors[i].a * (1.0 - accum.a);
    }
    out_color = accum;
})";

            GLuint compile_shader(GLenum type, const char *source) {
                std::string portable_source(source);
                size_t cursor = 0;
                while ((cursor = portable_source.find("layout(location=", cursor)) != std::string::npos) {
                    const size_t qualifier = portable_source.find(") ", cursor);
                    if (qualifier == std::string::npos)
                        break;
                    const std::string storage = portable_source.substr(qualifier + 2, 2);
                    if (storage != "in" && storage != "ou") {
                        cursor = qualifier + 2;
                        continue;
                    }
                    portable_source.erase(cursor, qualifier + 2 - cursor);
                }
                source = portable_source.c_str();
                const GLuint shader = glCreateShader(type);
                glShaderSource(shader, 1, &source, nullptr);
                glCompileShader(shader);
                GLint compiled = GL_FALSE;
                glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
                if (!compiled) {
                    GLint length = 0;
                    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
                    std::string log(length > 1 ? size_t(length) : 1, '\0');
                    if (length > 1)
                        glGetShaderInfoLog(shader, length, nullptr, &log[0]);
                    glDeleteShader(shader);
                    throw std::runtime_error("OpenGL RenderDevice shader compilation failed: " + log);
                }
                return shader;
            }

            GLuint create_program(int kind) {
                const char *vertex = kind == 1 ? point_vertex_shader : kind >= 2 ? line_vertex_shader : vertex_shader;
                const char *fragment = kind == 1 ? point_fragment_shader : kind == 2 ? line_fragment_shader :
                                       kind == 3 ? wide_line_fragment_shader : fragment_shader;
                const GLuint vertex_shader_id = compile_shader(GL_VERTEX_SHADER, vertex);
                GLuint fragment_shader_id = 0;
                try {
                    fragment_shader_id = compile_shader(GL_FRAGMENT_SHADER, fragment);
                } catch (...) {
                    glDeleteShader(vertex_shader_id);
                    throw;
                }
                const GLuint program = glCreateProgram();
                glAttachShader(program, vertex_shader_id);
                glAttachShader(program, fragment_shader_id);
                const char *attributes[] = {"position", "color", "uv", "normal", "highlight"};
                for (unsigned i = 0; i < 5; ++i)
                    glBindAttribLocation(program, i, attributes[i]);
                if (kind == 1) {
                    glBindAttribLocation(program, 5, "primitive_id");
                } else if (kind >= 2) {
                    const char *end_attributes[] = {"end_position", "end_color", "end_uv", "end_normal",
                                                    "end_highlight", "primitive_id"};
                    for (unsigned i = 0; i < 6; ++i)
                        glBindAttribLocation(program, 5 + i, end_attributes[i]);
                }
                glBindFragDataLocation(program, 0, "out_color");
                glBindFragDataLocation(program, 1, "out_id");
                glLinkProgram(program);
                glDeleteShader(vertex_shader_id);
                glDeleteShader(fragment_shader_id);
                GLint linked = GL_FALSE;
                glGetProgramiv(program, GL_LINK_STATUS, &linked);
                if (!linked) {
                    GLint length = 0;
                    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
                    std::string log(length > 1 ? size_t(length) : 1, '\0');
                    if (length > 1)
                        glGetProgramInfoLog(program, length, nullptr, &log[0]);
                    glDeleteProgram(program);
                    throw std::runtime_error("OpenGL RenderDevice shader linking failed: " + log);
                }
                return program;
            }

            GLuint create_effect_program(const char *vertex, const char *fragment, bool geometry, bool count = false) {
                const GLuint vertex_shader_id = compile_shader(GL_VERTEX_SHADER, vertex);
                GLuint fragment_shader_id = 0;
                try {
                    fragment_shader_id = compile_shader(GL_FRAGMENT_SHADER, fragment);
                } catch (...) {
                    glDeleteShader(vertex_shader_id);
                    throw;
                }
                const GLuint program = glCreateProgram();
                glAttachShader(program, vertex_shader_id);
                glAttachShader(program, fragment_shader_id);
                if (geometry) {
                    const char *attributes[] = {"position", "color", "uv", "normal", "highlight"};
                    for (unsigned i = 0; i < 5; ++i)
                        glBindAttribLocation(program, i, attributes[i]);
                } else {
                    // Effect quads reuse Geometry's attribute layout: position=0, uv=2.
                    glBindAttribLocation(program, 0, "position");
                    glBindAttribLocation(program, 2, "uv");
                }
                glBindFragDataLocation(program, 0, "out_color");
                if (count)
                    glBindFragDataLocation(program, 1, "out_count");
                glLinkProgram(program);
                glDeleteShader(vertex_shader_id);
                glDeleteShader(fragment_shader_id);
                GLint linked = GL_FALSE;
                glGetProgramiv(program, GL_LINK_STATUS, &linked);
                if (!linked) {
                    GLint length = 0;
                    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
                    std::string log(length > 1 ? size_t(length) : 1, '\0');
                    if (length > 1)
                        glGetProgramInfoLog(program, length, nullptr, &log[0]);
                    glDeleteProgram(program);
                    throw std::runtime_error("OpenGL RenderDevice effect shader linking failed: " + log);
                }
                return program;
            }

            struct GeometryResource {
                std::weak_ptr<const Geometry> owner;
                GLuint vao = 0, vertices = 0, indices = 0, primitive_buffer = 0, primitive_texture = 0;
            };

            struct TextureResource {
                std::weak_ptr<const Image> owner;
                GLuint texture = 0;
            };

            struct PointResource {
                std::weak_ptr<const PointData> owner;
            };

            struct PointProxy {
                Vertex vertex;
                uint32_t primitive;
            };

            struct LineProxy {
                Vertex start, end;
                uint32_t primitive;
            };

            struct ProxyResource {
                std::weak_ptr<const Geometry> owner;
                GLuint vao = 0, buffer = 0;
                uint32_t count = 0;
            };

            class ResourceCache {
            public:
                ~ResourceCache() {
                    clear();
                }

                void prune() {
                    for (auto it = geometries_.begin(); it != geometries_.end();) {
                        if (it->second.owner.expired())
                            it = erase_geometry(it);
                        else
                            ++it;
                    }
                    for (auto it = textures_.begin(); it != textures_.end();) {
                        if (it->second.owner.expired())
                            it = erase_texture(it);
                        else
                            ++it;
                    }
                    for (auto it = point_proxies_.begin(); it != point_proxies_.end();) {
                        if (it->second.owner.expired())
                            it = erase_proxy(point_proxies_, it);
                        else
                            ++it;
                    }
                    for (auto it = line_proxies_.begin(); it != line_proxies_.end();) {
                        if (it->second.owner.expired())
                            it = erase_proxy(line_proxies_, it);
                        else
                            ++it;
                    }
                    for (auto it = points_.begin(); it != points_.end();) {
                        if (it->second.owner.expired())
                            it = points_.erase(it);
                        else
                            ++it;
                    }
                }

                void clear() {
                    for (auto &entry : geometries_) {
                        glDeleteVertexArrays(1, &entry.second.vao);
                        glDeleteBuffers(1, &entry.second.vertices);
                        glDeleteBuffers(1, &entry.second.indices);
                        glDeleteBuffers(1, &entry.second.primitive_buffer);
                        glDeleteTextures(1, &entry.second.primitive_texture);
                    }
                    for (auto &entry : point_proxies_)
                        erase_proxy_value(entry.second);
                    for (auto &entry : line_proxies_)
                        erase_proxy_value(entry.second);
                    for (auto &entry : textures_)
                        glDeleteTextures(1, &entry.second.texture);
                    if (white_)
                        glDeleteTextures(1, &white_);
                    geometries_.clear();
                    textures_.clear();
                    point_proxies_.clear();
                    line_proxies_.clear();
                    points_.clear();
                    white_ = 0;
                }

                GeometryResource &geometry(const std::shared_ptr<const Geometry> &data) {
                    auto found = geometries_.find(data.get());
                    if (found != geometries_.end() && found->second.owner.lock() == data)
                        return found->second;
                    if (found != geometries_.end())
                        erase_geometry(found);
                    GeometryResource resource;
                    resource.owner = data;
                    glGenVertexArrays(1, &resource.vao);
                    glBindVertexArray(resource.vao);
                    glGenBuffers(1, &resource.vertices);
                    glBindBuffer(GL_ARRAY_BUFFER, resource.vertices);
                    const auto &vertices = data->vertices();
                    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
                    const size_t offsets[] = {offsetof(Vertex, position), offsetof(Vertex, color),
                                              offsetof(Vertex, texcoord), offsetof(Vertex, normal),
                                              offsetof(Vertex, highlight)};
                    const int sizes[] = {3, 4, 2, 3, 4};
                    for (unsigned i = 0; i < 5; ++i) {
                        glEnableVertexAttribArray(i);
                        glVertexAttribPointer(i, sizes[i], GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                              reinterpret_cast<const void *>(offsets[i]));
                    }
                    if (!data->indices().empty()) {
                        glGenBuffers(1, &resource.indices);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resource.indices);
                        glBufferData(GL_ELEMENT_ARRAY_BUFFER, data->indices().size() * sizeof(uint32_t),
                                     data->indices().data(), GL_STATIC_DRAW);
                    }
                    const uint32_t empty_id = 0;
                    const auto &primitive_ids = data->primitive_ids();
                    glGenBuffers(1, &resource.primitive_buffer);
                    glBindBuffer(GL_ARRAY_BUFFER, resource.primitive_buffer);
                    glBufferData(GL_ARRAY_BUFFER,
                                 GLsizeiptr((primitive_ids.empty() ? 1 : primitive_ids.size()) * sizeof(uint32_t)),
                                 primitive_ids.empty() ? &empty_id : primitive_ids.data(), GL_STATIC_DRAW);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glGenTextures(1, &resource.primitive_texture);
                    glBindTexture(GL_TEXTURE_BUFFER, resource.primitive_texture);
                    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, resource.primitive_buffer);
                    return geometries_.emplace(data.get(), std::move(resource)).first->second;
                }

                GLuint texture(const std::shared_ptr<const Image> &data) {
                    if (!data)
                        return white_texture();
                    auto found = textures_.find(data.get());
                    if (found != textures_.end() && found->second.owner.lock() == data)
                        return found->second.texture;
                    if (found != textures_.end())
                        erase_texture(found);
                    TextureResource resource;
                    resource.owner = data;
                    glGenTextures(1, &resource.texture);
                    glBindTexture(GL_TEXTURE_2D, resource.texture);
                    GLint unpack_buffer = 0;
                    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &unpack_buffer);
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                    const GLenum names[] = {GL_UNPACK_ALIGNMENT, GL_UNPACK_ROW_LENGTH,
                                            GL_UNPACK_SKIP_ROWS, GL_UNPACK_SKIP_PIXELS};
                    GLint saved[4];
                    for (unsigned i = 0; i < 4; ++i) {
                        glGetIntegerv(names[i], &saved[i]);
                        glPixelStorei(names[i], i ? 0 : 1);
                    }
                    const auto &rgba = data->rgba();
                    // Image rows are top-first; texture coordinate zero addresses the bottom row.
                    std::vector<unsigned char> flipped(rgba.size());
                    const size_t row_bytes = size_t(data->width()) * 4;
                    for (uint32_t y = 0; y < data->height(); ++y)
                        std::copy_n(rgba.data() + size_t(data->height() - 1 - y) * row_bytes, row_bytes,
                                    flipped.data() + size_t(y) * row_bytes);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, data->width(), data->height(), 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, flipped.data());
                    const auto sampler = data->sampler();
                    const GLint filter = sampler.filter == TextureFilter::linear ? GL_LINEAR : GL_NEAREST;
                    const GLint wrap = sampler.wrap == TextureWrap::repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
                    for (unsigned i = 0; i < 4; ++i)
                        glPixelStorei(names[i], saved[i]);
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpack_buffer);
                    return textures_.emplace(data.get(), std::move(resource)).first->second.texture;
                }

                void realize_points(const std::shared_ptr<const PointData> &points) {
                    if (!points)
                        return;
                    auto found = points_.find(points.get());
                    if (found != points_.end() && found->second.owner.lock() == points)
                        return;
                    points_[points.get()].owner = points;
                    ++uploads_;
                }

                ProxyResource &point_proxies(const std::shared_ptr<const Geometry> &data) {
                    auto found = point_proxies_.find(data.get());
                    if (found != point_proxies_.end() && found->second.owner.lock() == data)
                        return found->second;
                    if (found != point_proxies_.end())
                        erase_proxy(point_proxies_, found);
                    const auto &source = data->vertices();
                    const auto &indices = data->indices();
                    const uint32_t count = uint32_t(indices.empty() ? source.size() : indices.size());
                    std::vector<PointProxy> proxies;
                    proxies.reserve(count);
                    for (uint32_t i = 0; i < count; ++i) {
                        const Vertex &v = source[indices.empty() ? i : indices[i]];
                        const uint32_t id = data->primitive_ids().empty() ? i : data->primitive_ids()[i];
                        proxies.push_back({v, id});
                    }
                    ProxyResource resource = make_proxy<PointProxy>(data, proxies, false);
                    return point_proxies_.emplace(data.get(), std::move(resource)).first->second;
                }

                ProxyResource &line_proxies(const std::shared_ptr<const Geometry> &data) {
                    auto found = line_proxies_.find(data.get());
                    if (found != line_proxies_.end() && found->second.owner.lock() == data)
                        return found->second;
                    if (found != line_proxies_.end())
                        erase_proxy(line_proxies_, found);
                    const auto &source = data->vertices();
                    const auto &indices = data->indices();
                    const size_t count = (indices.empty() ? source.size() : indices.size()) / 2;
                    std::vector<LineProxy> proxies;
                    proxies.reserve(count);
                    for (size_t i = 0; i < count; ++i) {
                        const size_t a = indices.empty() ? i * 2 : indices[i * 2];
                        const size_t b = indices.empty() ? i * 2 + 1 : indices[i * 2 + 1];
                        const uint32_t id = data->primitive_ids().empty() ? uint32_t(i) : data->primitive_ids()[i];
                        proxies.push_back({source[a], source[b], id});
                    }
                    ProxyResource resource = make_proxy<LineProxy>(data, proxies, true);
                    return line_proxies_.emplace(data.get(), std::move(resource)).first->second;
                }

                std::uint64_t uploads() const {
                    return uploads_;
                }

            private:
                static void vertex_attributes(GLuint index, size_t offset, size_t stride) {
                    const size_t relative[] = {offsetof(Vertex, position), offsetof(Vertex, color),
                                               offsetof(Vertex, texcoord), offsetof(Vertex, normal),
                                               offsetof(Vertex, highlight)};
                    const int sizes[] = {3, 4, 2, 3, 4};
                    for (unsigned i = 0; i < 5; ++i) {
                        glEnableVertexAttribArray(index + i);
                        glVertexAttribPointer(index + i, sizes[i], GL_FLOAT, GL_FALSE, stride,
                                              reinterpret_cast<const void *>(offset + relative[i]));
                    }
                }

                template<typename T>
                ProxyResource make_proxy(const std::shared_ptr<const Geometry> &data,
                                         const std::vector<T> &proxies, bool lines) {
                    ProxyResource resource;
                    resource.owner = data;
                    resource.count = uint32_t(proxies.size());
                    glGenVertexArrays(1, &resource.vao);
                    glBindVertexArray(resource.vao);
                    glGenBuffers(1, &resource.buffer);
                    glBindBuffer(GL_ARRAY_BUFFER, resource.buffer);
                    glBufferData(GL_ARRAY_BUFFER, proxies.size() * sizeof(T), proxies.data(), GL_STATIC_DRAW);
                    vertex_attributes(0, lines ? size_t(0) : offsetof(PointProxy, vertex), sizeof(T));
                    for (unsigned i = 0; i < 5; ++i)
                        glVertexAttribDivisor(i, 1);
                    if (lines) {
                        vertex_attributes(5, offsetof(LineProxy, end), sizeof(T));
                        for (unsigned i = 5; i < 10; ++i)
                            glVertexAttribDivisor(i, 1);
                    }
                    glEnableVertexAttribArray(lines ? 10 : 5);
                    glVertexAttribIPointer(lines ? 10 : 5, 1, GL_UNSIGNED_INT, sizeof(T),
                                           reinterpret_cast<const void *>(offsetof(T, primitive)));
                    glVertexAttribDivisor(lines ? 10 : 5, 1);
                    glBindVertexArray(0);
                    return resource;
                }

                static void erase_proxy_value(ProxyResource &resource) {
                    glDeleteVertexArrays(1, &resource.vao);
                    glDeleteBuffers(1, &resource.buffer);
                }

                template<typename Map>
                static typename Map::iterator erase_proxy(Map &map, typename Map::iterator it) {
                    erase_proxy_value(it->second);
                    return map.erase(it);
                }

                std::unordered_map<const Geometry *, GeometryResource>::iterator
                erase_geometry(std::unordered_map<const Geometry *, GeometryResource>::iterator it) {
                    glDeleteVertexArrays(1, &it->second.vao);
                    glDeleteBuffers(1, &it->second.vertices);
                    glDeleteBuffers(1, &it->second.indices);
                    glDeleteBuffers(1, &it->second.primitive_buffer);
                    glDeleteTextures(1, &it->second.primitive_texture);
                    return geometries_.erase(it);
                }

                std::unordered_map<const Image *, TextureResource>::iterator
                erase_texture(std::unordered_map<const Image *, TextureResource>::iterator it) {
                    glDeleteTextures(1, &it->second.texture);
                    return textures_.erase(it);
                }

                GLuint white_texture() {
                    if (!white_) {
                        const unsigned char white[] = {255, 255, 255, 255};
                        glGenTextures(1, &white_);
                        glBindTexture(GL_TEXTURE_2D, white_);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    }
                    return white_;
                }

                std::unordered_map<const Geometry *, GeometryResource> geometries_;
                std::unordered_map<const Image *, TextureResource> textures_;
                std::unordered_map<const PointData *, PointResource> points_;
                std::unordered_map<const Geometry *, ProxyResource> point_proxies_;
                std::unordered_map<const Geometry *, ProxyResource> line_proxies_;
                GLuint white_ = 0;
                std::uint64_t uploads_ = 0;
            };

            class FrameTarget {
            public:
                void ensure(uint32_t width, uint32_t height, unsigned samples) {
                    const int stored_samples = samples > 1 ? int(samples) : 0;
                    if (scene_ && scene_->width() == int(width) && scene_->height() == int(height) &&
                        output_ && output_->width() == int(width) && output_->height() == int(height) &&
                        color_ && color_->width() == int(width) && color_->height() == int(height) &&
                        color_->samples() == stored_samples)
                        return;
                    scene_.reset();
                    output_.reset();
                    color_.reset();
                    scene_ = std::unique_ptr<FramebufferObject>(new FramebufferObject(int(width), int(height), 0));
                    if (!scene_->add_color_texture() ||
                        !scene_->add_color_texture(GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT) ||
                        !scene_->add_depth_texture(GL_DEPTH_COMPONENT32F) || !scene_->is_valid())
                        throw std::runtime_error("cannot allocate OpenGL RenderDevice picking target");
                    scene_->activate_draw_buffers(0, 1);
                    color_ = std::unique_ptr<FramebufferObject>(new FramebufferObject(int(width), int(height), int(samples)));
                    if (samples > 1) {
                        if (!color_->add_color_buffer() || !color_->add_depth_buffer(GL_DEPTH_COMPONENT32F) ||
                            color_->samples() != int(samples) || !color_->is_valid())
                            throw std::runtime_error("cannot allocate requested OpenGL RenderDevice MSAA target");
                    } else if (!color_->add_color_texture() || !color_->add_depth_buffer(GL_DEPTH_COMPONENT32F) ||
                               !color_->is_valid()) {
                        throw std::runtime_error("cannot allocate OpenGL RenderDevice color target");
                    }
                    output_ = std::unique_ptr<FramebufferObject>(new FramebufferObject(int(width), int(height), 0));
                    if (!output_->add_color_texture() ||
                        !output_->attach_depth_texture(GL_TEXTURE_2D, scene_->depth_texture(), GL_DEPTH_ATTACHMENT) ||
                        !output_->is_valid())
                        throw std::runtime_error("cannot allocate OpenGL RenderDevice output target");
                }

                FramebufferObject &color() const {
                    return *color_;
                }
                FramebufferObject &scene() const {
                    return *scene_;
                }
                FramebufferObject &output() const {
                    return *output_;
                }

            private:
                std::unique_ptr<FramebufferObject> color_, scene_, output_;
            };

            // Private effect scratch targets. They are reallocated only when the
            // frame shape or an explicit effect parameter changes, and never appear
            // in the public RenderDevice API.
            class EffectResources {
            public:
                std::unique_ptr<FramebufferObject> shadow, ao_raw, ao_blur, average, accumulate;
                std::unique_ptr<FramebufferObject> peel[2], linked_build;
                GLuint head = 0, nodes = 0, counter = 0;
                uint32_t width = 0, height = 0, layers = 0;
                uint32_t shadow_width = 0, shadow_height = 0;
                unsigned shadow_size = 0, shadow_samples = 0;
                bool shadow_soft = false;

                ~EffectResources() {
                    if (head) glDeleteBuffers(1, &head);
                    if (nodes) glDeleteBuffers(1, &nodes);
                    if (counter) glDeleteBuffers(1, &counter);
                }

                void reset_shadow(uint32_t frame_width, uint32_t frame_height, unsigned size,
                                  bool soft, unsigned samples) {
                    if (shadow && shadow_width == frame_width && shadow_height == frame_height &&
                        shadow_size == size && shadow_soft == soft && shadow_samples == samples)
                        return;
                    shadow.reset(new FramebufferObject(int(size), int(size), 0));
                    if (!shadow->add_depth_texture(GL_DEPTH_COMPONENT32F, GL_LINEAR) || !shadow->is_valid())
                        throw std::runtime_error("cannot allocate OpenGL RenderDevice shadow map");
                    shadow->deactivate_draw_buffers();
                    shadow_width = frame_width;
                    shadow_height = frame_height;
                    shadow_size = size;
                    shadow_soft = soft;
                    shadow_samples = samples;
                }

                void reset_ao(uint32_t w, uint32_t h) {
                    if (ao_raw && ao_raw->width() == int(w) && ao_raw->height() == int(h) &&
                        ao_blur && ao_blur->width() == int(w) && ao_blur->height() == int(h))
                        return;
                    ao_raw.reset(new FramebufferObject(int(w), int(h), 0));
                    ao_blur.reset(new FramebufferObject(int(w), int(h), 0));
                    if (!ao_raw->add_color_texture(GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR) || !ao_raw->is_valid() ||
                        !ao_blur->add_color_texture(GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR) || !ao_blur->is_valid())
                        throw std::runtime_error("cannot allocate OpenGL RenderDevice ambient-occlusion targets");
                }

                void reset_average(uint32_t w, uint32_t h) {
                    if (average && average->width() == int(w) && average->height() == int(h))
                        return;
                    average.reset(new FramebufferObject(int(w), int(h), 0));
                    if (!average->add_color_texture(GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST) ||
                        !average->add_color_texture(GL_R32F, GL_RED, GL_FLOAT, GL_NEAREST) || !average->is_valid())
                        throw std::runtime_error("cannot allocate OpenGL RenderDevice average transparency target");
                    average->activate_draw_buffers(0, 1);
                }

                void reset_peel(uint32_t w, uint32_t h) {
                    // Peel at 2x for silhouette coverage; accumulation stays at output resolution.
                    if (peel[0] && peel[0]->width() == int(w) * 2 && peel[0]->height() == int(h) * 2 &&
                        peel[1] && peel[1]->height() == int(h) * 2)
                        return;
                    for (int i = 0; i < 2; ++i) {
                        peel[i].reset(new FramebufferObject(int(w) * 2, int(h) * 2, 0));
                        if (!peel[i]->add_color_texture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, GL_NEAREST) ||
                            !peel[i]->add_depth_texture(GL_DEPTH_COMPONENT32F, GL_NEAREST) || !peel[i]->is_valid())
                            throw std::runtime_error("cannot allocate OpenGL RenderDevice depth-peeling targets");
                    }
                }

                void reset_accumulate(uint32_t w, uint32_t h) {
                    if (accumulate && accumulate->width() == int(w) && accumulate->height() == int(h))
                        return;
                    accumulate.reset(new FramebufferObject(int(w), int(h), 0));
                    if (!accumulate->add_color_texture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, GL_NEAREST) ||
                        !accumulate->is_valid())
                        throw std::runtime_error("cannot allocate OpenGL RenderDevice transparency target");
                }

                void reset_linked(uint32_t w, uint32_t h, unsigned max_fragments) {
                    const size_t pixels = size_t(w) * h;
                    const size_t node_count = pixels * std::max(1u, max_fragments);
                    if (head && width == w && height == h && layers == max_fragments)
                        return;
                    if (head) glDeleteBuffers(1, &head);
                    if (nodes) glDeleteBuffers(1, &nodes);
                    if (counter) glDeleteBuffers(1, &counter);
                    width = w;
                    height = h;
                    layers = max_fragments;
                    glGenBuffers(1, &head);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, head);
                    glBufferData(GL_SHADER_STORAGE_BUFFER, GLsizeiptr(pixels * sizeof(int)), nullptr, GL_DYNAMIC_DRAW);
                    glGenBuffers(1, &nodes);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodes);
                    glBufferData(GL_SHADER_STORAGE_BUFFER, GLsizeiptr(node_count * 32), nullptr, GL_DYNAMIC_DRAW);
                    glGenBuffers(1, &counter);
                    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, counter);
                    glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
                    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
                    if (!linked_build || linked_build->width() != int(w) || linked_build->height() != int(h)) {
                        linked_build.reset(new FramebufferObject(int(w), int(h), 0));
                        if (!linked_build->add_color_texture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_NEAREST) ||
                            !linked_build->add_depth_buffer(GL_DEPTH_COMPONENT32F) || !linked_build->is_valid())
                            throw std::runtime_error("cannot allocate OpenGL RenderDevice linked-list target");
                    }
                }

                uint32_t node_count() const { return width * height * std::max(1u, layers); }
            };

            bool finite(const vec3 &v) {
                for (int i = 0; i < 3; ++i)
                    if (!std::isfinite(v[i]))
                        return false;
                return true;
            }

            bool finite(const vec4 &v) {
                for (int i = 0; i < 4; ++i)
                    if (!std::isfinite(v[i]))
                        return false;
                return true;
            }

            bool finite(const mat4 &m) {
                for (int i = 0; i < 16; ++i)
                    if (!std::isfinite(m[i]))
                        return false;
                return true;
            }

            bool finite(const mat3 &m) {
                for (int i = 0; i < 9; ++i)
                    if (!std::isfinite(m[i]))
                        return false;
                return true;
            }

            GLenum topology(Topology value) {
                return value == Topology::triangles ? GL_TRIANGLES : value == Topology::lines ? GL_LINES : GL_POINTS;
            }

            ivec4 viewport_of(const DrawCommand &command, uint32_t width, uint32_t height) {
                ivec4 result = command.viewport;
                if (result == ivec4(0))
                    return ivec4(0, 0, int(width), int(height));
                if (result.x < 0 || result.y < 0 || result.z <= 0 || result.w <= 0 ||
                    int64_t(result.x) + int64_t(result.z) > int64_t(width) ||
                    int64_t(result.y) + int64_t(result.w) > int64_t(height))
                    throw std::invalid_argument("draw viewport is outside the render target");
                return result;
            }

            bool on_segment(const vec2 &a, const vec2 &b, const vec2 &p) {
                return std::abs((b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x)) < 1e-4f &&
                       p.x >= std::min(a.x, b.x) - 1e-4f && p.x <= std::max(a.x, b.x) + 1e-4f &&
                       p.y >= std::min(a.y, b.y) - 1e-4f && p.y <= std::max(a.y, b.y) + 1e-4f;
            }

            bool inside_polygon(const vec2 &p, const std::vector<vec2> &polygon) {
                bool inside = false;
                for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
                    const vec2 &a = polygon[j], &b = polygon[i];
                    if (on_segment(a, b, p))
                        return true;
                    if ((a.y > p.y) != (b.y > p.y) &&
                        p.x < (b.x - a.x) * (p.y - a.y) / (b.y - a.y) + a.x)
                        inside = !inside;
                }
                return inside;
            }

        } // namespace

        struct OpenGLDevice::Impl {
            std::string name, driver;
            mutable unsigned sample_mask = 0;
            mutable bool sample_mask_ready = false;
            GLuint program = 0, point_program = 0, line_program = 0, wide_line_program = 0;
            GLuint texture_program = 0, depth_copy_program = 0, shadow_depth_program = 0, shadow_program = 0;
            GLuint ao_program = 0, ao_blur_program = 0, darkness_program = 0, edl_program = 0;
            GLuint oit_program = 0, oit_peel_program = 0, oit_composite_program = 0;
            GLuint linked_program = 0, linked_resolve_program = 0;
            ResourceCache resources;
            FrameTarget target;
            EffectResources effects;
            opengl::OverlayRenderer overlays;
            TransparencyMethod transparency = TransparencyMethod::sorted;
            unsigned peel_layers = 4;
            std::uint32_t dropped_fragments = 0;
            int gl_major = 0, gl_minor = 0;

            ~Impl() {
                const GLuint programs[] = {point_program, line_program, wide_line_program, program,
                                           texture_program, depth_copy_program, shadow_depth_program, shadow_program, ao_program,
                                           ao_blur_program, darkness_program, edl_program, oit_program,
                                           oit_peel_program, oit_composite_program, linked_program,
                                           linked_resolve_program};
                for (GLuint p : programs)
                    if (p)
                        glDeleteProgram(p);
            }

            bool linked_lists_supported() const {
                return gl_major > 4 || (gl_major == 4 && gl_minor >= 3);
            }

            void ensure_effect_programs(bool linked = false) {
                if (!texture_program)
                    texture_program = create_effect_program(effect_vertex_shader, texture_composite_fragment_shader,
                                                            false);
                if (!depth_copy_program)
                    depth_copy_program = create_effect_program(effect_vertex_shader, depth_copy_fragment_shader,
                                                               false);
                if (!shadow_depth_program)
                    shadow_depth_program = create_effect_program(shadow_depth_vertex_shader, empty_fragment_shader,
                                                                 false);
                if (!shadow_program)
                    shadow_program = create_effect_program(effect_vertex_shader, shadow_fragment_shader, false);
                if (!ao_program)
                    ao_program = create_effect_program(effect_vertex_shader, ao_fragment_shader, false);
                if (!ao_blur_program)
                    ao_blur_program = create_effect_program(effect_vertex_shader, ao_blur_fragment_shader, false);
                if (!darkness_program)
                    darkness_program = create_effect_program(effect_vertex_shader, darkness_composite_fragment_shader,
                                                             false);
                if (!edl_program)
                    edl_program = create_effect_program(effect_vertex_shader, edl_fragment_shader, false);
                if (!oit_program)
                    oit_program = create_effect_program(vertex_shader, oit_fragment_shader, true, true);
                if (!oit_peel_program)
                    oit_peel_program = create_effect_program(vertex_shader, oit_peel_fragment_shader, true);
                if (!oit_composite_program)
                    oit_composite_program = create_effect_program(effect_vertex_shader, oit_composite_fragment_shader,
                                                                  false);
                if (linked && linked_lists_supported()) {
                    if (!linked_program)
                        linked_program = create_effect_program(linked_vertex_shader, linked_build_fragment_shader, true);
                    if (!linked_resolve_program)
                        linked_resolve_program = create_effect_program(linked_quad_vertex_shader,
                                                                      linked_resolve_fragment_shader, false);
                }
            }

            void require_context() {
                const GLubyte *version = glGetString(GL_VERSION);
                if (!version)
                    throw std::runtime_error("OpenGL RenderDevice requires a current context");
            }

            [[noreturn]] void reject(const char *message) {
                throw std::invalid_argument(message);
            }

            void validate(uint32_t width, uint32_t height, unsigned samples, TransparencyMethod transparency) {
                if (!width || !height || width > uint32_t(std::numeric_limits<int>::max()) ||
                    height > uint32_t(std::numeric_limits<int>::max()))
                    reject("render target dimensions must be positive and representable");
                GLint viewport[2] = {0, 0};
                glGetIntegerv(GL_MAX_VIEWPORT_DIMS, viewport);
                if (int(width) > viewport[0] || int(height) > viewport[1])
                    reject("render target exceeds OpenGL viewport limits");
                if (transparency == TransparencyMethod::peeling &&
                    (width > uint32_t(viewport[0]) / 2 || height > uint32_t(viewport[1]) / 2))
                    reject("depth peeling requires twice the render target resolution within OpenGL viewport limits");
                switch (transparency) {
                    case TransparencyMethod::sorted:
                    case TransparencyMethod::average:
                    case TransparencyMethod::peeling:
                    case TransparencyMethod::linked_lists:
                        break;
                    default:
                        reject("unknown transparency method");
                }
                if (!samples || (samples & (samples - 1)) || !(capabilities() & samples))
                    reject("unsupported OpenGL MSAA sample count");
            }

            void validate(const DrawCommand &d, uint32_t width, uint32_t height) {
                if (!d.geometry)
                    reject("draw command has no geometry");
                viewport_of(d, width, height);
                if (!std::isfinite(d.depth_range().x) || !std::isfinite(d.depth_range().y) ||
                    d.depth_range().x < 0 || d.depth_range().x > d.depth_range().y || d.depth_range().y > 1)
                    reject("draw depth range must be an ordered interval in [0,1]");
                if (!finite(d.model_view_projection) || !finite(d.normal_matrix) || !finite(d.model) ||
                    !finite(d.model_view) || !finite(d.projection) ||
                    !finite(d.eye) || !finite(d.light) || !finite(d.ambient) || !finite(d.specular) ||
                    !finite(d.back_color) || !finite(d.color) || !finite(d.shadow_matrix))
                    reject("draw command contains non-finite rendering state");
                if (determinant(d.model_view) == 0 || determinant(d.projection) == 0)
                    reject("impostor view and projection matrices must be invertible");
                if (d.model_view(3, 0) != 0 || d.model_view(3, 1) != 0 || d.model_view(3, 2) != 0 ||
                    d.model_view(3, 3) != 1)
                    reject("impostor view matrix must be affine");
                if (d.transparent && (d.color.w < 0 || d.color.w > 1))
                    reject("transparent draw alpha must be in [0,1]");
                if (d.overlay_depth_test && !d.overlay)
                    reject("scene-depth annotation requires overlay=true");
                if (d.overlay_independent_depth && (!d.overlay || d.overlay_depth_test))
                    reject("independent-depth HUD requires overlay=true and excludes scene-depth testing");
                const bool depth_effect = d.scene_depth_texture || d.eye_dome_lighting || d.ambient_occlusion || d.shadow;
                if (depth_effect &&
                    (!d.overlay || d.overlay_depth_test || d.overlay_independent_depth || d.texture))
                    reject("full-screen depth effects require a flat overlay without an image texture");
                if (d.eye_dome_lighting &&
                    (!std::isfinite(d.color.x) || !std::isfinite(d.color.y) || !std::isfinite(d.color.z) ||
                     d.color.x <= 0 || d.color.y <= d.color.x || d.color.z <= 0))
                    reject("eye-dome lighting requires a positive near/far/scene range");
                if (d.ambient_occlusion &&
                    (!std::isfinite(d.color.x) || d.color.x <= 0 || !std::isfinite(d.color.y) || d.color.y < 0 ||
                     !std::isfinite(d.color.z) || d.color.z < 0 || d.color.z > 1 || !finite(d.projection)))
                    reject("ambient occlusion requires radius, bias, intensity, and a finite projection");
                if (d.shadow &&
                    (!std::isfinite(d.color.x) || d.color.x < 0 || d.color.x > 1 ||
                     !std::isfinite(d.color.y) || d.color.y < 0 ||
                     !std::isfinite(d.color.z) || !std::isfinite(d.color.w) ||
                     d.color.z <= 0 || d.color.w <= d.color.z))
                    reject("shadow requires darkness in [0,1], non-negative softness, and a light near/far range");
                if ((d.shadow || d.shadow_caster || d.shadow_background) &&
                    (!d.shadow_map_size || (d.shadow_map_size & (d.shadow_map_size - 1))))
                    reject("shadow map size must be a nonzero power of two");
                if ((d.shadow || d.shadow_caster || d.shadow_background)) {
                    GLint max_size = 0;
                    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);
                    if (d.shadow_map_size > unsigned(std::max(0, max_size)))
                        reject("shadow map size exceeds the OpenGL texture limit");
                }
                if ((d.shadow_caster || d.shadow_background) && d.geometry->topology() != Topology::triangles)
                    reject("shadow casters and receivers require triangle geometry");
                if (d.shadow_background && (d.overlay || d.object_id != UINT32_MAX))
                    reject("shadow background requires a scene draw with the background ID");
                // Analytic point/line draws carry clipping through their fragment shaders
                // (see the point/line impostor programs below); plain points and lines clip
                // geometrically through the shared CPU path in prepare().
                if (d.overlay && d.geometry->topology() != Topology::triangles)
                    reject("overlays require triangle geometry");
                if (d.transparent && (!d.overlay && d.geometry->topology() != Topology::triangles))
                    reject("transparent commands require scene triangles");
                if (d.overlay && d.transparent)
                    reject("overlay commands cannot also be transparent scene geometry");
                if (!std::isfinite(d.point_size) || d.point_size <= 0)
                    reject("point size must be positive and finite");
                if (!std::isfinite(d.line_width) || d.line_width <= 0)
                    reject("line width must be positive and finite");
                if (d.geometry->topology() == Topology::points && d.point_style == PointStyle::surfel) {
                    const auto &vertices = d.geometry->vertices();
                    const auto &indices = d.geometry->indices();
                    const size_t count = indices.empty() ? vertices.size() : indices.size();
                    for (size_t i = 0; i < count; ++i) {
                        const vec3 &normal = vertices[indices.empty() ? i : indices[i]].normal;
                        if (!finite(normal) || dot(normal, normal) == 0)
                            reject("surfel normals must be finite and nonzero");
                    }
                }
                if (d.geometry->topology() == Topology::points && d.point_style != PointStyle::plain &&
                    (!std::isfinite(d.sphere_radius) || d.sphere_radius <= 0))
                    reject("point impostor radius must be positive and finite");
                if (d.geometry->topology() == Topology::lines &&
                    ((d.line_style != LineStyle::plain && (!std::isfinite(d.line_radius) || d.line_radius <= 0)) ||
                     (d.line_style == LineStyle::plain && !d.line_screen_space_width &&
                      (!std::isfinite(d.line_radius) || d.line_radius <= 0))))
                    reject("line radius must be positive and finite");
                if (!d.overlay && !d.shadow_background && d.object_id == UINT32_MAX)
                    reject("UINT32_MAX is reserved for the background object ID");
            }

            bool float_blending_supported() const {
                // Blending to 32-bit float attachments is core from OpenGL 4.4. On
                // earlier contexts it requires an advertised extension; core
                // floating-point color buffers alone are not sufficient.
                return glewIsSupported("GL_ARB_color_buffer_float") ||
                       glewIsSupported("GL_EXT_color_buffer_float") ||
                       glewIsSupported("GL_EXT_float_blend") ||
                       gl_major > 4 || (gl_major == 4 && gl_minor >= 4);
            }

            unsigned capabilities() {
                if (sample_mask_ready)
                    return sample_mask;
                opengl::QueryState state;
                GLint previous_draw = 0, previous_read = 0, previous_renderbuffer = 0;
                glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previous_draw);
                glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previous_read);
                glGetIntegerv(GL_RENDERBUFFER_BINDING, &previous_renderbuffer);
                sample_mask = 1;
                GLint maximum = 0;
                glGetIntegerv(GL_MAX_SAMPLES, &maximum);
                for (unsigned samples = 2; samples <= unsigned(maximum); samples *= 2) {
                    GLuint fbo = 0, color = 0, depth = 0;
                    glGenFramebuffers(1, &fbo);
                    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
                    glGenRenderbuffers(1, &color);
                    glBindRenderbuffer(GL_RENDERBUFFER, color);
                    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, 4, 4);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
                    glGenRenderbuffers(1, &depth);
                    glBindRenderbuffer(GL_RENDERBUFFER, depth);
                    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT32F, 4, 4);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
                    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
                        sample_mask |= samples;
                    glDeleteFramebuffers(1, &fbo);
                    glDeleteRenderbuffers(1, &color);
                    glDeleteRenderbuffers(1, &depth);
                }
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GLuint(previous_draw));
                glBindFramebuffer(GL_READ_FRAMEBUFFER, GLuint(previous_read));
                glBindRenderbuffer(GL_RENDERBUFFER, GLuint(previous_renderbuffer));
                sample_mask_ready = true;
                return sample_mask;
            }

            DrawCommand expand_triangle(const DrawCommand &d, size_t triangle) {
                const auto &source = d.geometry->vertices();
                const auto &indices = d.geometry->indices();
                std::vector<Vertex> vertices;
                for (size_t i = 0; i < 3; ++i) {
                    const size_t element = triangle * 3 + i;
                    vertices.push_back(source[indices.empty() ? element : indices[element]]);
                }
                std::vector<uint32_t> ids{uint32_t(triangle)};
                if (!d.geometry->primitive_ids().empty())
                    ids[0] = d.geometry->primitive_ids()[triangle];
                DrawCommand expanded = d;
                expanded.geometry = std::make_shared<Geometry>(std::move(vertices), std::vector<uint32_t>{0, 1, 2},
                                                              Topology::triangles, std::move(ids));
                return expanded;
            }

            std::vector<DrawCommand> prepare(const std::vector<DrawCommand> &commands, uint32_t width, uint32_t height) {
                std::vector<DrawCommand> opaque, transparent;
                for (auto d : commands) {
                    if (!d.clipping_planes.empty() || !d.object_clipping_planes.empty())
                        d.clipping_planes = graphics::normalize_clipping_planes(d.clipping_planes);
                    if (d.overlay)
                        continue;
                    if (!d.clipping_planes.empty() || !d.object_clipping_planes.empty())
                        d = internal::clip_draw(d);
                    if (d.transparent) {
                        const size_t count = d.geometry->indices().empty() ? d.geometry->vertices().size()
                                                                            : d.geometry->indices().size();
                        for (size_t i = 0; i < count / 3; ++i)
                            transparent.push_back(expand_triangle(d, i));
                    } else {
                        opaque.push_back(d);
                    }
                }
                auto depth = [](const DrawCommand &d) {
                    const auto &v = d.geometry->vertices();
                    const vec3 center = (v[0].position + v[1].position + v[2].position) / 3.0f;
                    const vec4 clip = d.model_view_projection * vec4(center, 1);
                    if (clip.w <= 0)
                        return std::numeric_limits<float>::infinity();
                    const float window_depth = 0.5f * (clip.z / clip.w + 1.0f);
                    return d.depth_range().x + (d.depth_range().y - d.depth_range().x) * window_depth;
                };
                std::stable_sort(transparent.begin(), transparent.end(), [&](const DrawCommand &a, const DrawCommand &b) {
                    return depth(a) > depth(b);
                });
                opaque.insert(opaque.end(), std::make_move_iterator(transparent.begin()),
                              std::make_move_iterator(transparent.end()));
                return opaque;
            }

            void clear_scene(const vec4 &background) {
                glDisable(GL_SCISSOR_TEST);
                auto &scene = target.scene();
                scene.bind();
                scene.activate_draw_buffers(0, 1);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                const GLuint background_id[] = {UINT32_MAX, UINT32_MAX, 0, 0};
                glClearBufferuiv(GL_COLOR, 1, background_id);
                glClearDepth(1);
                glClear(GL_DEPTH_BUFFER_BIT);
                auto &color = target.color();
                color.bind();
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glClearColor(background.x, background.y, background.z, background.w);
                glClearDepth(1);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            }

            void set_uniforms(GLuint p, const DrawCommand &d, bool ignore_highlight = false) {
                auto location = [p](const char *name) { return glGetUniformLocation(p, name); };
                glUniformMatrix4fv(location("mvp"), 1, GL_FALSE, static_cast<const float *>(d.model_view_projection));
                glUniformMatrix4fv(location("model"), 1, GL_FALSE, static_cast<const float *>(d.model));
                glUniformMatrix3fv(location("normal_matrix"), 1, GL_FALSE, static_cast<const float *>(d.normal_matrix));
                // Scene programs name the material color `color`; impostor programs
                // name the per-vertex tint `tint`. OIT programs use both. Uniform
                // locations that do not exist are ignored by OpenGL.
                glUniform4fv(location("color"), 1, d.color.data());
                glUniform4fv(location("tint"), 1, d.color.data());
                glUniform3fv(location("eye"), 1, d.eye.data());
                glUniform3fv(location("light"), 1, d.light.data());
                glUniform4fv(location("ambient"), 1, d.ambient.data());
                glUniform4fv(location("specular"), 1, d.specular.data());
                glUniform4fv(location("back_color"), 1, d.back_color.data());
                glUniform1f(location("shininess"), d.shininess);
                glUniform1i(location("lighting"), d.lighting);
                glUniform1i(location("material_lighting"), d.material_lighting);
                glUniform1i(location("two_sides"), d.lighting_two_sides);
                glUniform1i(location("distinct_back_color"), d.distinct_back_color);
                glUniform1i(location("ignore_highlight"), ignore_highlight);
                glUniform1ui(location("object_id"), d.object_id);
                const bool has_ids = !d.geometry->primitive_ids().empty();
                glUniform1i(location("has_primitive_ids"), has_ids);
                glUniform1i(location("image"), 0);
                glUniform1i(location("primitive_ids"), 1);
            }

            void set_impostor_uniforms(const DrawCommand &d, const ivec4 &viewport,
                                       GLuint p, bool color_pass, unsigned samples, uint32_t height) {
                auto location = [p](const char *name) { return glGetUniformLocation(p, name); };
                const bool points = d.geometry->topology() == Topology::points;
                const float radius = points ? d.sphere_radius : d.line_radius;
                const vec4 dimensions(viewport.z, viewport.w, points ? d.point_size : d.line_width, radius);
                const vec4 origin(viewport.x, int(height) - viewport.y - viewport.w, 0, 0);
                const mat4 inverse_projection = inverse(d.projection);
                glUniformMatrix4fv(location("model_view"), 1, GL_FALSE, static_cast<const float *>(d.model_view));
                glUniformMatrix4fv(location("projection"), 1, GL_FALSE, static_cast<const float *>(d.projection));
                glUniformMatrix4fv(location("inverse_projection"), 1, GL_FALSE, static_cast<const float *>(inverse_projection));
                glUniform4fv(location("dimensions"), 1, dimensions.data());
                glUniform4fv(location("viewport_origin"), 1, origin.data());
                glUniform1i(location("style"), points ? int(d.point_style) : 3 + int(d.line_style));
                glUniform1i(location("single_sample"), !color_pass || samples <= 1);
                glUniform3fv(location("sphere_light"), 1, d.sphere_light.data());
                glUniform1i(location("screen_space"), d.line_screen_space_width);
                glUniform1i(location("distinct_back_color"), d.distinct_back_color);
                glUniform4fv(location("back_color"), 1, d.back_color.data());
                const vec2 depth_range = d.depth_range();
                glUniform2fv(location("depth_range"), 1, depth_range.data());
                std::vector<float> clip_data(8, 0.0f);
                for (unsigned i = 0; i < d.clipping_planes.size() && i < 2; ++i) {
                    clip_data[4 * i + 0] = d.clipping_planes[i].x;
                    clip_data[4 * i + 1] = d.clipping_planes[i].y;
                    clip_data[4 * i + 2] = d.clipping_planes[i].z;
                    clip_data[4 * i + 3] = d.clipping_planes[i].w;
                }
                glUniform4fv(location("clip_planes"), 2, clip_data.data());
                glUniform1i(location("clip_count"), int(d.clipping_planes.size()));
            }

            void draw(const DrawCommand &d, uint32_t width, uint32_t height, bool color_pass,
                      bool transparent, unsigned samples) {
                const ivec4 viewport = viewport_of(d, width, height);
                const int x = viewport.x, y = int(height) - viewport.y - viewport.w;
                const bool points = d.geometry->topology() == Topology::points;
                const bool lines = d.geometry->topology() == Topology::lines;
                const bool analytic = points ? d.point_style != PointStyle::plain
                                             : lines && (d.line_style != LineStyle::plain ||
                                                         d.line_width != 1 || !d.line_screen_space_width);
                glViewport(x, y, viewport.z, viewport.w);
                // Analytic shaders write final window depth; viewport mapping applies to proxy vertices.
                glDepthRange(analytic ? 0 : d.depth_range().x, analytic ? 0 : d.depth_range().y);
                glEnable(GL_SCISSOR_TEST);
                glScissor(x, y, viewport.z, viewport.w);
                glDepthMask(GL_TRUE);
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                if (!color_pass) {
                    glColorMaski(0, GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    glColorMaski(1, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                } else {
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                }
                if (transparent && color_pass) {
                    glEnable(GL_BLEND);
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                } else {
                    glDisable(GL_BLEND);
                }
                if (d.depth_bias && d.geometry->topology() == Topology::triangles) {
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(0.5f, -0.0001f);
                } else {
                    glDisable(GL_POLYGON_OFFSET_FILL);
                }
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, resources.texture(d.texture));
                if (points)
                    resources.realize_points(d.geometry->point_data());
                if (!analytic) {
                    glUseProgram(program);
                    set_uniforms(program, d);
                    auto &resource = resources.geometry(d.geometry);
                    glBindVertexArray(resource.vao);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_BUFFER, resource.primitive_texture);
                    glActiveTexture(GL_TEXTURE0);
                    if (points)
                        glPointSize(d.point_size);
                    const GLenum mode = topology(d.geometry->topology());
                    if (resource.indices)
                        glDrawElements(mode, GLsizei(d.geometry->indices().size()), GL_UNSIGNED_INT, nullptr);
                    else
                        glDrawArrays(mode, 0, GLsizei(d.geometry->vertices().size()));
                    return;
                }
                if (color_pass && samples > 1) {
                    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
                    glEnable(GL_MULTISAMPLE);
                } else {
                    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
                }
                const GLuint p = points ? point_program :
                                 lines && d.line_style == LineStyle::plain ? wide_line_program : line_program;
                glUseProgram(p);
                set_uniforms(p, d);
                set_impostor_uniforms(d, viewport, p, color_pass, samples, height);
                auto &proxy = points ? resources.point_proxies(d.geometry) : resources.line_proxies(d.geometry);
                glBindVertexArray(proxy.vao);
                glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GLsizei(proxy.count));
                glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
            }

            void draw_scene(const std::vector<DrawCommand> &commands, uint32_t width, uint32_t height,
                            bool include_color, bool include_ids, unsigned samples) {
                glUseProgram(program);
                glDisable(GL_CULL_FACE);
                glEnable(GL_DEPTH_TEST);
                if (include_color) {
                    target.color().bind();
                    for (const auto &d : commands)
                        draw(d, width, height, true, d.transparent, samples);
                }
                if (include_ids) {
                    target.scene().bind();
                    target.scene().activate_draw_buffers(0, 1);
                    for (const auto &d : commands)
                        draw(d, width, height, false, false, 1);
                }
                glDisable(GL_SCISSOR_TEST);
            }

            void draw_overlay_batch(const std::vector<DrawCommand> &commands, uint32_t width, uint32_t height) {
                if (commands.empty())
                    return;
                auto &output = target.color();
                output.bind();
                output.activate_draw_buffer(0);
                overlays.set(commands, false);
                overlays.draw(false, int(width), int(height), ivec4(0), {});
            }

            void composite_effect(const DrawCommand &d, uint32_t width, uint32_t height) {
                if (d.shadow && effects.shadow)
                    composite_shadow(d, width, height);
                else if (d.ambient_occlusion)
                    composite_ao(d, width, height);
                else if (d.eye_dome_lighting)
                    composite_edl(d, width, height);
                else if (d.scene_depth_texture)
                    composite_scene_depth(d, width, height);
            }

            void draw_overlays(const std::vector<DrawCommand> &commands, uint32_t width, uint32_t height,
                               bool include_effects) {
                std::vector<DrawCommand> depth_tested, independent;
                for (const auto &d : commands) {
                    if (!d.overlay)
                        continue;
                    if (d.overlay_depth_test)
                        depth_tested.push_back(d);
                    else if (d.overlay_independent_depth)
                        independent.push_back(d);
                }
                // Vulkan renders scene-depth annotations, then independent-depth HUD,
                // then flat overlays and full-screen effects in submitted order.
                draw_overlay_batch(depth_tested, width, height);
                draw_overlay_batch(independent, width, height);
                std::vector<DrawCommand> flat;
                const auto flush = [&]() {
                    draw_overlay_batch(flat, width, height);
                    flat.clear();
                };
                for (const auto &d : commands) {
                    if (!d.overlay || d.overlay_depth_test || d.overlay_independent_depth)
                        continue;
                    const bool effect = d.shadow || d.ambient_occlusion || d.eye_dome_lighting || d.scene_depth_texture;
                    if (effect && include_effects) {
                        flush();
                        composite_effect(d, width, height);
                    } else if (!effect) {
                        flat.push_back(d);
                    }
                }
                flush();
            }

            DrawCommand fullscreen_quad(uint32_t width, uint32_t height) {
                static std::shared_ptr<Geometry> geometry;
                if (!geometry) {
                    std::vector<Vertex> vertices(6);
                    const vec2 points[] = {vec2(0, 0), vec2(1, 0), vec2(1, 1),
                                           vec2(0, 0), vec2(1, 1), vec2(0, 1)};
                    for (unsigned i = 0; i < 6; ++i) {
                        vertices[i].position = vec3(points[i], 0);
                        vertices[i].texcoord = points[i];
                    }
                    geometry = std::make_shared<Geometry>(std::move(vertices));
                }
                DrawCommand result;
                result.geometry = geometry;
                // The quad is normalized to [0, 1], so it needs a normalized screen
                // projection. graphics::screen_space_projection() maps pixel coordinates
                // and would cover only one pixel of this quad.
                mat4 projection = mat4::identity();
                projection(0, 0) = 2.0f;
                projection(1, 1) = -2.0f;
                projection(0, 3) = -1.0f;
                projection(1, 3) = 1.0f;
                result.model_view_projection = projection;
                return result;
            }

            void bind_2d(GLenum unit, GLuint texture) {
                glActiveTexture(unit);
                glBindTexture(GL_TEXTURE_2D, texture);
                glActiveTexture(GL_TEXTURE0);
            }

            void set_resolution(GLuint program, uint32_t width, uint32_t height) {
                glUniform2f(glGetUniformLocation(program, "resolution"), float(width), float(height));
            }

            void draw_fullscreen(GLuint p, const DrawCommand &d, uint32_t width, uint32_t height) {
                glUseProgram(p);
                set_resolution(p, width, height);
                glUniformMatrix4fv(glGetUniformLocation(p, "mvp"), 1, GL_FALSE,
                                   static_cast<const float *>(d.model_view_projection));
                auto &resource = resources.geometry(d.geometry);
                glBindVertexArray(resource.vao);
                const ivec4 viewport = viewport_of(d, width, height);
                glDisable(GL_SCISSOR_TEST);
                glViewport(viewport.x, int(height) - viewport.y - viewport.w, viewport.z, viewport.w);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                const auto &indices = d.geometry->indices();
                if (indices.empty())
                    glDrawArrays(GL_TRIANGLES, 0, GLsizei(d.geometry->vertices().size()));
                else
                    glDrawElements(GL_TRIANGLES, GLsizei(indices.size()), GL_UNSIGNED_INT, nullptr);
            }

            // Write the scene depth texture into the currently bound draw
            // framebuffer without a depth blit: the default framebuffer's depth
            // format is driver-defined and need not be blit-compatible.
            void copy_depth(uint32_t width, uint32_t height, GLuint depth_texture) {
                ensure_effect_programs();
                const auto quad = fullscreen_quad(width, height);
                GLint previous_program = 0, previous_vao = 0, previous_depth_func = 0;
                GLboolean previous_depth_mask = GL_FALSE;
                GLboolean previous_color_mask[4] = {GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE};
                glGetIntegerv(GL_CURRENT_PROGRAM, &previous_program);
                glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previous_vao);
                glGetIntegerv(GL_DEPTH_FUNC, &previous_depth_func);
                glGetBooleanv(GL_DEPTH_WRITEMASK, &previous_depth_mask);
                glGetBooleanv(GL_COLOR_WRITEMASK, previous_color_mask);

                glUseProgram(depth_copy_program);
                set_resolution(depth_copy_program, width, height);
                glUniformMatrix4fv(glGetUniformLocation(depth_copy_program, "mvp"), 1, GL_FALSE,
                                   static_cast<const float *>(quad.model_view_projection));
                glUniform1i(glGetUniformLocation(depth_copy_program, "scene_depth"), 0);
                auto &resource = resources.geometry(quad.geometry);
                glBindVertexArray(resource.vao);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, depth_texture);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_ALWAYS);
                glDepthMask(GL_TRUE);
                glDisable(GL_SCISSOR_TEST);
                glViewport(0, 0, int(width), int(height));
                glDrawArrays(GL_TRIANGLES, 0, 6);

                glBindVertexArray(previous_vao);
                glUseProgram(previous_program);
                glDepthFunc(previous_depth_func);
                glDepthMask(previous_depth_mask);
                glColorMask(previous_color_mask[0], previous_color_mask[1], previous_color_mask[2],
                            previous_color_mask[3]);
            }

            void draw_geometry(const DrawCommand &d, GLuint p, uint32_t width, uint32_t height,
                               unsigned viewport_scale = 1) {
                glUseProgram(p);
                set_uniforms(p, d);
                bind_2d(GL_TEXTURE0, resources.texture(d.texture));
                auto &resource = resources.geometry(d.geometry);
                glBindVertexArray(resource.vao);
                // OIT prepasses draw into full-target attachments. Scene commands may
                // carry a cell viewport, so apply the same top-left command viewport
                // and scissor as the ordinary scene pass; fullscreen composite quads
                // are drawn separately by draw_fullscreen().
                const ivec4 viewport = viewport_of(d, width, height);
                const int x = viewport.x * int(viewport_scale);
                const int y = int(height) * int(viewport_scale) - viewport.y * int(viewport_scale) -
                              viewport.w * int(viewport_scale);
                const int w = viewport.z * int(viewport_scale), h = viewport.w * int(viewport_scale);
                glViewport(x, y, w, h);
                glEnable(GL_SCISSOR_TEST);
                glScissor(x, y, w, h);
                const auto &indices = d.geometry->indices();
                if (indices.empty())
                    glDrawArrays(topology(d.geometry->topology()), 0,
                                 GLsizei(d.geometry->vertices().size()));
                else
                    glDrawElements(topology(d.geometry->topology()), GLsizei(indices.size()),
                                   GL_UNSIGNED_INT, nullptr);
            }

            void render_shadow_map(const std::vector<DrawCommand> &scene, unsigned map_size,
                                   uint32_t width, uint32_t height, unsigned samples, bool soft) {
                effects.reset_shadow(width, height, map_size, soft, samples);
                auto &map = *effects.shadow;
                map.bind();
                map.deactivate_draw_buffers();
                glViewport(0, 0, int(map_size), int(map_size));
                glDisable(GL_SCISSOR_TEST);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                glDepthMask(GL_TRUE);
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                glClearDepth(1);
                glClear(GL_DEPTH_BUFFER_BIT);
                glUseProgram(shadow_depth_program);
                for (const auto &d : scene) {
                    if (!d.shadow_caster)
                        continue;
                    const mat4 mvp = d.shadow_matrix * d.model_view_projection;
                    glUniformMatrix4fv(glGetUniformLocation(shadow_depth_program, "mvp"), 1, GL_FALSE,
                                       static_cast<const float *>(mvp));
                    auto &resource = resources.geometry(d.geometry);
                    glBindVertexArray(resource.vao);
                    const auto &indices = d.geometry->indices();
                    if (indices.empty())
                        glDrawArrays(GL_TRIANGLES, 0, GLsizei(d.geometry->vertices().size()));
                    else
                        glDrawElements(GL_TRIANGLES, GLsizei(indices.size()), GL_UNSIGNED_INT, nullptr);
                }
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            }

            void composite_shadow(const DrawCommand &d, uint32_t width, uint32_t height) {
                target.color().bind();
                target.color().activate_draw_buffer(0);
                glEnable(GL_BLEND);
                // Match Vulkan: shadow darkening preserves the accumulated alpha.
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                                    GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                glUseProgram(shadow_program);
                bind_2d(GL_TEXTURE0, target.scene().depth_texture());
                bind_2d(GL_TEXTURE1, effects.shadow->depth_texture());
                glUniform1i(glGetUniformLocation(shadow_program, "scene_depth"), 0);
                glUniform1i(glGetUniformLocation(shadow_program, "shadow_map"), 1);
                glUniformMatrix4fv(glGetUniformLocation(shadow_program, "shadow_matrix"), 1, GL_FALSE,
                                   static_cast<const float *>(d.shadow_matrix));
                glUniform4fv(glGetUniformLocation(shadow_program, "effect"), 1, d.color.data());
                draw_fullscreen(shadow_program, d, width, height);
            }

            void composite_ao(const DrawCommand &d, uint32_t width, uint32_t height) {
                effects.reset_ao(width, height);
                glDisable(GL_BLEND);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                effects.ao_raw->bind();
                effects.ao_raw->activate_draw_buffer(0);
                glViewport(0, 0, int(width), int(height));
                glDisable(GL_SCISSOR_TEST);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                float one[] = {1, 1, 1, 1};
                glClearBufferfv(GL_COLOR, 0, one);
                glUseProgram(ao_program);
                bind_2d(GL_TEXTURE0, target.scene().depth_texture());
                glUniform1i(glGetUniformLocation(ao_program, "scene_depth"), 0);
                glUniformMatrix4fv(glGetUniformLocation(ao_program, "projection"), 1, GL_FALSE,
                                   static_cast<const float *>(d.projection));
                const mat4 inverse_projection = inverse(d.projection);
                glUniformMatrix4fv(glGetUniformLocation(ao_program, "inverse_projection"), 1, GL_FALSE,
                                   static_cast<const float *>(inverse_projection));
                glUniform4fv(glGetUniformLocation(ao_program, "effect"), 1, d.color.data());
                draw_fullscreen(ao_program, d, width, height);

                effects.ao_blur->bind();
                effects.ao_blur->activate_draw_buffer(0);
                glViewport(0, 0, int(width), int(height));
                glClearBufferfv(GL_COLOR, 0, one);
                glUseProgram(ao_blur_program);
                bind_2d(GL_TEXTURE0, target.scene().depth_texture());
                bind_2d(GL_TEXTURE1, effects.ao_raw->color_texture(0));
                glUniform1i(glGetUniformLocation(ao_blur_program, "scene_depth"), 0);
                glUniform1i(glGetUniformLocation(ao_blur_program, "ao_raw"), 1);
                glUniformMatrix4fv(glGetUniformLocation(ao_blur_program, "inverse_projection"), 1, GL_FALSE,
                                   static_cast<const float *>(inverse_projection));
                glUniform4fv(glGetUniformLocation(ao_blur_program, "effect"), 1, d.color.data());
                draw_fullscreen(ao_blur_program, d, width, height);

                target.color().bind();
                target.color().activate_draw_buffer(0);
                glEnable(GL_BLEND);
                // Match Vulkan: AO/EDL darkening leaves destination alpha intact.
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                                    GL_ZERO, GL_ONE);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                glUseProgram(darkness_program);
                bind_2d(GL_TEXTURE0, effects.ao_blur->color_texture(0));
                glUniform1i(glGetUniformLocation(darkness_program, "factor"), 0);
                draw_fullscreen(darkness_program, d, width, height);
            }

            void composite_edl(const DrawCommand &d, uint32_t width, uint32_t height) {
                target.color().bind();
                target.color().activate_draw_buffer(0);
                glEnable(GL_BLEND);
                glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                                    GL_ZERO, GL_ONE);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                glUseProgram(edl_program);
                bind_2d(GL_TEXTURE0, target.scene().depth_texture());
                glUniform1i(glGetUniformLocation(edl_program, "scene_depth"), 0);
                glUniform4fv(glGetUniformLocation(edl_program, "effect"), 1, d.color.data());
                draw_fullscreen(edl_program, d, width, height);
            }

            void composite_scene_depth(const DrawCommand &d, uint32_t width, uint32_t height) {
                target.color().bind();
                target.color().activate_draw_buffer(0);
                glDisable(GL_BLEND);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                glUseProgram(texture_program);
                bind_2d(GL_TEXTURE0, target.scene().depth_texture());
                glUniform1i(glGetUniformLocation(texture_program, "image"), 0);
                draw_fullscreen(texture_program, d, width, height);
            }

            void composite_oit(GLuint texture, uint32_t width, uint32_t height, bool use_count) {
                target.color().bind();
                target.color().activate_draw_buffer(0);
                glEnable(GL_BLEND);
                glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                glUseProgram(oit_composite_program);
                bind_2d(GL_TEXTURE0, texture);
                // Average-color OIT stores the per-pixel fragment count in the second
                // color attachment; peeling and linked lists resolve to premultiplied
                // color that needs no count texture.
                bind_2d(GL_TEXTURE1, use_count ? effects.average->color_texture(1) : texture);
                glUniform1i(glGetUniformLocation(oit_composite_program, "accumulated"), 0);
                glUniform1i(glGetUniformLocation(oit_composite_program, "fragment_count"), 1);
                glUniform1i(glGetUniformLocation(oit_composite_program, "use_count"), use_count ? 1 : 0);
                glUniform1i(glGetUniformLocation(oit_composite_program, "supersample"), 0);
                draw_fullscreen(oit_composite_program, fullscreen_quad(width, height), width, height);
            }

            struct OitResult {
                GLuint texture = 0;
                bool use_count = false;
            };

            OitResult accumulate_oit(const std::vector<DrawCommand> &scene, uint32_t width, uint32_t height,
                                     TransparencyMethod method, unsigned layers) {
                OitResult result;
                std::vector<const DrawCommand *> transparent;
                for (const auto &d : scene)
                    if (d.transparent)
                        transparent.push_back(&d);
                if (transparent.empty())
                    return result;
                ensure_effect_programs(method == TransparencyMethod::linked_lists);
                glDisable(GL_SCISSOR_TEST);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                if (method == TransparencyMethod::average) {
                    effects.reset_average(width, height);
                    effects.average->bind();
                    effects.average->activate_draw_buffers(0, 1);
                    glViewport(0, 0, int(width), int(height));
                    float zero[4] = {0, 0, 0, 0};
                    glClearBufferfv(GL_COLOR, 0, zero);
                    glClearBufferfv(GL_COLOR, 1, zero);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_ONE, GL_ONE);
                    for (const DrawCommand *d : transparent)
                        draw_geometry(*d, oit_program, width, height);
                    result.texture = effects.average->color_texture(0);
                    result.use_count = true;
                } else if (method == TransparencyMethod::linked_lists) {
                    effects.reset_linked(width, height, layers);
                    GLint previous_ssbo[2] = {0, 0}, previous_atomic = 0;
                    GLint previous_generic_ssbo = 0, previous_generic_atomic = 0;
                    glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 0, &previous_ssbo[0]);
                    glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, 1, &previous_ssbo[1]);
                    glGetIntegeri_v(GL_ATOMIC_COUNTER_BUFFER_BINDING, 0, &previous_atomic);
                    glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &previous_generic_ssbo);
                    glGetIntegerv(GL_ATOMIC_COUNTER_BUFFER_BINDING, &previous_generic_atomic);
                    const int empty = -1;
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, effects.head);
                    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32I, GL_RED_INTEGER, GL_INT, &empty);
                    const uint32_t zero = 0;
                    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, effects.counter);
                    glClearBufferData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, effects.head);
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, effects.nodes);
                    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, effects.counter);
                    effects.linked_build->bind();
                    effects.linked_build->activate_draw_buffer(0);
                    glViewport(0, 0, int(width), int(height));
                    glClearColor(0, 0, 0, 0);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    glDisable(GL_BLEND);
                    glUseProgram(linked_program);
                    glUniform1ui(glGetUniformLocation(linked_program, "max_nodes"), effects.node_count());
                    glUniform1ui(glGetUniformLocation(linked_program, "viewport_width"), width);
                    for (const DrawCommand *d : transparent)
                        draw_geometry(*d, linked_program, width, height);
                    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT |
                                    GL_BUFFER_UPDATE_BARRIER_BIT);
                    uint32_t attempts = 0;
                    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, effects.counter);
                    glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(attempts), &attempts);
                    const uint32_t capacity = effects.node_count();
                    dropped_fragments = attempts > capacity ? attempts - capacity : 0;
                    effects.reset_accumulate(width, height);
                    effects.accumulate->bind();
                    effects.accumulate->activate_draw_buffer(0);
                    glViewport(0, 0, int(width), int(height));
                    float zero_color[4] = {0, 0, 0, 0};
                    glClearBufferfv(GL_COLOR, 0, zero_color);
                    glUseProgram(linked_resolve_program);
                    glUniform1ui(glGetUniformLocation(linked_resolve_program, "viewport_width"), width);
                    draw_fullscreen(linked_resolve_program, fullscreen_quad(width, height), width, height);
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, GLuint(previous_ssbo[0]));
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, GLuint(previous_ssbo[1]));
                    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, GLuint(previous_atomic));
                    glBindBuffer(GL_SHADER_STORAGE_BUFFER, GLuint(previous_generic_ssbo));
                    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, GLuint(previous_generic_atomic));
                    result.texture = effects.accumulate->color_texture(0);
                } else if (method == TransparencyMethod::peeling) {
                    const uint32_t peel_width = width * 2, peel_height = height * 2;
                    effects.reset_peel(width, height);
                    effects.reset_accumulate(width, height);
                    for (auto &layer : effects.peel) {
                        layer->bind();
                        layer->activate_draw_buffer(0);
                        glViewport(0, 0, int(peel_width), int(peel_height));
                        glDisable(GL_SCISSOR_TEST);
                        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                        // Enable writes so the zero-depth sentinel clear takes effect.
                        glDepthMask(GL_TRUE);
                        glClearDepth(0);
                        glClear(GL_DEPTH_BUFFER_BIT);
                        glClearDepth(1);
                    }
                    glDepthMask(GL_FALSE);
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    effects.accumulate->bind();
                    effects.accumulate->activate_draw_buffer(0);
                    glViewport(0, 0, int(width), int(height));
                    glDisable(GL_SCISSOR_TEST);
                    float zero_color[4] = {0, 0, 0, 0};
                    glClearBufferfv(GL_COLOR, 0, zero_color);
                    for (unsigned i = 0; i < layers; ++i) {
                        const unsigned target = i & 1u, previous = 1u - target;
                        effects.peel[target]->bind();
                        effects.peel[target]->activate_draw_buffer(0);
                        glViewport(0, 0, int(peel_width), int(peel_height));
                        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                        glDepthMask(GL_TRUE);
                        glClearColor(0, 0, 0, 0);
                        glClearDepth(1);
                        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                        glDisable(GL_BLEND);
                        glEnable(GL_DEPTH_TEST);
                        glDepthFunc(GL_LESS);
                        // Each iteration keeps one nearest surviving fragment per sample.
                        glUseProgram(oit_peel_program);
                        bind_2d(GL_TEXTURE1, effects.peel[previous]->depth_texture());
                        glUniform1i(glGetUniformLocation(oit_peel_program, "previous_depth"), 1);
                        for (const DrawCommand *d : transparent) {
                            glDepthRange(d->depth_range().x, d->depth_range().y);
                            draw_geometry(*d, oit_peel_program, width, height, 2);
                        }
                        glDepthRange(0, 1);
                        effects.accumulate->bind();
                        effects.accumulate->activate_draw_buffer(0);
                        glViewport(0, 0, int(width), int(height));
                        glDisable(GL_SCISSOR_TEST);
                        glEnable(GL_BLEND);
                        glBlendEquation(GL_FUNC_ADD);
                        glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE,
                                            GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                        glDisable(GL_DEPTH_TEST);
                        glDepthMask(GL_FALSE);
                        glUseProgram(oit_composite_program);
                        bind_2d(GL_TEXTURE0, effects.peel[target]->color_texture(0));
                        glUniform1i(glGetUniformLocation(oit_composite_program, "accumulated"), 0);
                        glUniform1i(glGetUniformLocation(oit_composite_program, "use_count"), 0);
                        glUniform1i(glGetUniformLocation(oit_composite_program, "supersample"), 1);
                        draw_fullscreen(oit_composite_program, fullscreen_quad(width, height), width, height);
                    }
                    result.texture = effects.accumulate->color_texture(0);
                }
                glDisable(GL_BLEND);
                glDisable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                return result;
            }

            void render(uint32_t width, uint32_t height, const std::vector<DrawCommand> &commands,
                        const vec4 &background, unsigned samples, FrameReadback readback,
                        bool overlays, bool include_color, bool pixel_query,
                        TransparencyMethod method, unsigned layers) {
                require_context();
                drain_errors();
                resources.prune();
                dropped_fragments = 0;
                // The viewer validates settings values; this device is the authority on
                // backend support. Capability checks precede every native/fallback
                // choice so unsupported methods degrade with an explicit one-time
                // diagnostic instead of being rejected before the device can explain.
                transparency = method;
                if (method == TransparencyMethod::linked_lists && !linked_lists_supported()) {
                    LOG_N_TIMES(1, WARNING) << "OpenGL per-pixel linked lists require OpenGL 4.3; falling back to depth peeling";
                    transparency = TransparencyMethod::peeling;
                } else if (method == TransparencyMethod::average && !float_blending_supported()) {
                    LOG_N_TIMES(1, WARNING) << "OpenGL average-color blending requires floating-point color blending; falling back to sorted transparency";
                    transparency = TransparencyMethod::sorted;
                }
                validate(width, height, samples, transparency);
                for (const auto &d : commands) {
                    if (!pixel_query || !d.overlay)
                        validate(d, width, height);
                }
                peel_layers = layers ? layers : 16;
                if (!program)
                    program = create_program(0);
                if (!point_program)
                    point_program = create_program(1);
                if (!line_program)
                    line_program = create_program(2);
                if (!wide_line_program)
                    wide_line_program = create_program(3);
                target.ensure(width, height, samples);
                clear_scene(background);
                const auto scene = prepare(commands, width, height);
                const bool present_effects = include_color && !pixel_query;
                OitResult oit;
                if (present_effects) {
                    ensure_effect_programs(transparency == TransparencyMethod::linked_lists &&
                                          linked_lists_supported());
                    unsigned shadow_size = 0;
                    bool shadow_soft = false;
                    for (const auto &d : commands) {
                        if ((d.shadow || d.shadow_caster || d.shadow_background) && d.shadow_map_size > shadow_size)
                            shadow_size = d.shadow_map_size;
                        shadow_soft = shadow_soft || (d.shadow && d.color.y > 0);
                    }
                    if (shadow_size)
                        render_shadow_map(scene, shadow_size, width, height, samples, shadow_soft);
                    if (transparency != TransparencyMethod::sorted)
                        oit = accumulate_oit(scene, width, height, transparency, peel_layers);
                    check_error();
                }
                // The single-sample ID/depth target is the authoritative scene depth for
                // picking, AO, EDL, and shadow lookup. It is produced before color and
                // never receives effect overlays.
                draw_scene(scene, width, height, false, true, 1);
                if (include_color) {
                    if (transparency == TransparencyMethod::sorted || !present_effects)
                        draw_scene(scene, width, height, true, false, samples);
                    else {
                        std::vector<DrawCommand> opaque;
                        for (const auto &d : scene)
                            if (!d.transparent)
                                opaque.push_back(d);
                        draw_scene(opaque, width, height, true, false, samples);
                    }
                    // A frame containing only transparent commands performs no opaque
                    // color pass, so the preceding ID/depth pass leaves color writes
                    // masked off. Compositing must restore them explicitly.
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    if (oit.texture)
                        composite_oit(oit.texture, width, height, oit.use_count);
                    glViewport(0, 0, int(width), int(height));
                    glScissor(0, 0, int(width), int(height));
                    if (overlays)
                        draw_overlays(commands, width, height, present_effects);
                    check_error();
                    glDisable(GL_SCISSOR_TEST);
                    auto &color = target.color();
                    auto &output = target.output();
                    color.bind(GL_READ_FRAMEBUFFER);
                    output.bind(GL_DRAW_FRAMEBUFFER);
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, color.handle());
                    glReadBuffer(GL_COLOR_ATTACHMENT0);
                    glBlitFramebuffer(0, 0, int(width), int(height), 0, 0, int(width), int(height),
                                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
                    check_error();
                }
            }

            FrameImage capture(uint32_t width, uint32_t height, FrameReadback readback) {
                FrameImage result;
                result.width = width;
                result.height = height;
                const size_t pixels = size_t(width) * height;
                auto &output = target.output();
                output.bind(GL_READ_FRAMEBUFFER);
                glReadBuffer(GL_COLOR_ATTACHMENT0);
                result.rgba.resize(pixels * 4);
                for (uint32_t y = 0; y < height; ++y)
                    glReadPixels(0, int(height - 1 - y), int(width), 1, GL_RGBA, GL_UNSIGNED_BYTE,
                                 result.rgba.data() + size_t(y) * width * 4);
                if (readback == FrameReadback::color_depth_ids) {
                    auto &scene = target.scene();
                    scene.bind(GL_READ_FRAMEBUFFER);
                    glReadBuffer(GL_COLOR_ATTACHMENT1);
                    std::vector<uint32_t> ids(pixels * 2);
                    for (uint32_t y = 0; y < height; ++y)
                        glReadPixels(0, int(height - 1 - y), int(width), 1, GL_RG_INTEGER, GL_UNSIGNED_INT,
                                     ids.data() + size_t(y) * width * 2);
                    result.ids.resize(pixels);
                    for (size_t i = 0; i < pixels; ++i)
                        result.ids[i] = {ids[2 * i], ids[2 * i + 1]};
                    result.depth.resize(pixels);
                    for (uint32_t y = 0; y < height; ++y)
                        glReadPixels(0, int(height - 1 - y), int(width), 1, GL_DEPTH_COMPONENT, GL_FLOAT,
                                     result.depth.data() + size_t(y) * width);
                }
                return result;
            }

            void drain_errors() {
                while (glGetError() != GL_NO_ERROR) {}
            }

            void check_error() {
                const GLenum error = glGetError();
                if (error == GL_NO_ERROR)
                    return;
                // Drain the queue so a later call cannot misattribute a stale error.
                drain_errors();
                throw std::runtime_error("OpenGL RenderDevice encountered GL error " + std::to_string(error));
            }
        };

        OpenGLDevice::OpenGLDevice() : impl_(new Impl) {
            impl_->require_context();
            const GLubyte *renderer = glGetString(GL_RENDERER);
            const GLubyte *version = glGetString(GL_VERSION);
            impl_->name = renderer ? std::string("OpenGL (") + reinterpret_cast<const char *>(renderer) + ")" : "OpenGL";
            impl_->driver = version ? reinterpret_cast<const char *>(version) : "";
            GLint major = 0, minor = 0;
            glGetIntegerv(GL_MAJOR_VERSION, &major);
            glGetIntegerv(GL_MINOR_VERSION, &minor);
            if (major < 3 || (major == 3 && minor < 2))
                throw std::runtime_error("OpenGL RenderDevice requires OpenGL 3.2 or later");
            impl_->gl_major = major;
            impl_->gl_minor = minor;
        }

        OpenGLDevice::~OpenGLDevice() = default;

        const std::string &OpenGLDevice::name() const {
            return impl_->name;
        }

        const std::string &OpenGLDevice::driver_version() const {
            return impl_->driver;
        }

        std::uint32_t OpenGLDevice::supported_sample_counts(PointStyle) const {
            impl_->require_context();
            return impl_->capabilities();
        }

        std::uint32_t OpenGLDevice::supported_sample_counts(LineStyle) const {
            impl_->require_context();
            return impl_->capabilities();
        }

        FrameImage OpenGLDevice::render(uint32_t width, uint32_t height, const std::vector<DrawCommand> &commands,
                                        const vec4 &background, unsigned samples, TransparencyMethod transparency,
                                        unsigned peel_layers, FrameReadback readback) {
            if (readback != FrameReadback::color && readback != FrameReadback::color_depth_ids)
                impl_->reject("unknown frame readback mode");
            opengl::QueryState state;
            state.prepare(int(width), int(height));
            impl_->render(width, height, commands, background, samples, readback, true, true, false,
                          transparency, peel_layers);
            FrameImage result = impl_->capture(width, height, readback);
            impl_->check_error();
            return result;
        }

        ScenePixel OpenGLDevice::sample_pixel(uint32_t width, uint32_t height,
                                              const std::vector<DrawCommand> &commands, uint32_t x, uint32_t y) {
            if (x >= width || y >= height)
                impl_->reject("pixel query is outside the render target");
            opengl::QueryState state;
            state.prepare(int(width), int(height));
            impl_->render(width, height, commands, vec4(0), 1, FrameReadback::color_depth_ids, false, false, true,
                          TransparencyMethod::sorted, 0);
            auto &scene = impl_->target.scene();
            scene.bind(GL_READ_FRAMEBUFFER);
            glReadBuffer(GL_COLOR_ATTACHMENT1);
            GLuint id[2] = {UINT32_MAX, UINT32_MAX};
            GLfloat depth = 1;
            glReadPixels(int(x), int(height - 1 - y), 1, 1, GL_RG_INTEGER, GL_UNSIGNED_INT, id);
            glReadPixels(int(x), int(height - 1 - y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
            impl_->check_error();
            ScenePixel result;
            result.id = {id[0], id[1]};
            result.depth = depth;
            return result;
        }

        std::vector<FrameImage::Id> OpenGLDevice::sample_ids(uint32_t width, uint32_t height,
                                                             const std::vector<DrawCommand> &commands,
                                                             uint32_t x, uint32_t y,
                                                             uint32_t region_width, uint32_t region_height) {
            if (!region_width || !region_height || x >= width || y >= height ||
                uint64_t(x) + region_width > width || uint64_t(y) + region_height > height) {
                throw std::out_of_range("ID query region is outside the render target");
            }
            opengl::QueryState state;
            state.prepare(int(width), int(height));
            impl_->render(width, height, commands, vec4(0), 1, FrameReadback::color_depth_ids, false, false, true,
                          TransparencyMethod::sorted, 0);
            auto &scene = impl_->target.scene();
            scene.bind(GL_READ_FRAMEBUFFER);
            glReadBuffer(GL_COLOR_ATTACHMENT1);
            std::vector<uint32_t> ids(size_t(region_width) * region_height * 2);
            for (uint32_t row = 0; row < region_height; ++row)
                glReadPixels(int(x), int(height - 1 - y - row), int(region_width), 1,
                             GL_RG_INTEGER, GL_UNSIGNED_INT, ids.data() + size_t(row) * region_width * 2);
            impl_->check_error();
            std::vector<FrameImage::Id> result(size_t(region_width) * region_height);
            for (size_t i = 0; i < result.size(); ++i)
                result[i] = {ids[2 * i], ids[2 * i + 1]};
            return result;
        }

        bool OpenGLDevice::present(uint32_t width, uint32_t height, const std::vector<DrawCommand> &commands,
                                   const vec4 &background, unsigned samples, TransparencyMethod transparency,
                                   unsigned peel_layers) {
            opengl::QueryState state;
            state.prepare(int(width), int(height));
            GLint destination_samples = 0;
            if (state.draw_fbo) {
                GLint type = 0, buffer = 0;
                glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
                glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                      GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &buffer);
                if (type == GL_RENDERBUFFER) {
                    GLint bound = 0;
                    glGetIntegerv(GL_RENDERBUFFER_BINDING, &bound);
                    glBindRenderbuffer(GL_RENDERBUFFER, buffer);
                    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &destination_samples);
                    glBindRenderbuffer(GL_RENDERBUFFER, bound);
                }
            } else
                glGetIntegerv(GL_SAMPLES, &destination_samples);
            const GLenum destination_error = glGetError();
            if (destination_error != GL_NO_ERROR)
                throw std::runtime_error("OpenGL presentation destination query failed: " +
                                         std::to_string(destination_error));
            const unsigned target_samples = destination_samples > 1 && samples <= 1
                                            ? unsigned(destination_samples) : (samples > 1 ? samples : 1u);
            const auto check = [&](const char *stage) {
                const GLenum error = glGetError();
                if (error != GL_NO_ERROR)
                    throw std::runtime_error(std::string("OpenGL presentation failed at ") + stage + ": " +
                                             std::to_string(error));
            };
            impl_->render(width, height, commands, background, target_samples, FrameReadback::color, true, true, false,
                          transparency, peel_layers);
            check("scene rendering");
            auto &output = impl_->target.output();
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GLenum(state.draw_fbo));
            // Keep sample coverage intact when the caller already has a compatible
            // target; resolving first would decorrelate later MSAA HUD composition.
            const bool compatible = destination_samples == int(target_samples > 1 ? target_samples : 0);
            auto &source = compatible ? impl_->target.color() : output;
            source.bind(GL_READ_FRAMEBUFFER);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GLenum(state.draw_fbo));
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            check("source binding");
            glDisable(GL_SCISSOR_TEST);
            if (state.draw_fbo == 0) {
                glBlitFramebuffer(0, 0, int(width), int(height), 0, 0, int(width), int(height),
                                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
                check("default color blit");
                // Depth cannot be blit portably: the default framebuffer's depth
                // format is driver-defined and need not match our FBO. Rasterize
                // the scene depth into it instead, preserving depth for native
                // overlays and queries regardless of sample count or format.
                impl_->copy_depth(width, height, impl_->target.scene().depth_texture());
                check("default depth write");
            } else {
                glBlitFramebuffer(0, 0, int(width), int(height), 0, 0, int(width), int(height),
                                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
                check("color blit");
                const auto depth_format = [&](GLuint fbo, unsigned sample_count) -> GLenum {
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
                    GLenum attachment = GL_DEPTH_ATTACHMENT;
                    GLint type = 0;
                    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachment,
                                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
                    if (type == GL_NONE) {
                        attachment = GL_DEPTH_STENCIL_ATTACHMENT;
                        glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachment,
                                                              GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
                    }
                    if (type == GL_NONE)
                        return GL_NONE;
                    GLint object = 0, format = 0, bound = 0;
                    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, attachment,
                                                          GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &object);
                    if (type == GL_RENDERBUFFER) {
                        glGetIntegerv(GL_RENDERBUFFER_BINDING, &bound);
                        glBindRenderbuffer(GL_RENDERBUFFER, object);
                        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_INTERNAL_FORMAT, &format);
                        glBindRenderbuffer(GL_RENDERBUFFER, bound);
                    } else if (type == GL_TEXTURE && sample_count == 0) {
                        glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound);
                        glBindTexture(GL_TEXTURE_2D, object);
                        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
                        glBindTexture(GL_TEXTURE_2D, bound);
                    } else {
                        return GL_NONE;
                    }
                    return GLenum(format);
                };
                const bool multisample_depth = compatible && target_samples > 1;
                auto &depth = multisample_depth ? impl_->target.color() : impl_->target.scene();
                const GLenum destination_format = depth_format(GLuint(state.draw_fbo), unsigned(destination_samples));
                const GLenum source_format = depth_format(depth.handle(), multisample_depth ? target_samples : 0);
                check("depth format query");
                if (destination_format != GL_NONE && destination_format == source_format) {
                    depth.bind(GL_READ_FRAMEBUFFER);
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GLenum(state.draw_fbo));
                    glBlitFramebuffer(0, 0, int(width), int(height), 0, 0, int(width), int(height),
                                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);
                    check("depth blit");
                }
            }
            return true;
        }

        unsigned OpenGLDevice::validation_errors() const {
            return 0;
        }

        std::vector<uint32_t> OpenGLDevice::select_points(const PointSelectionRequest &request) {
            impl_->require_context();
            if (!request.width || !request.height || !request.points || request.polygon.empty())
                impl_->reject("invalid point selection request");
            const size_t expected_words = (request.points->source_count() + 31u) / 32u;
            if (!request.words.empty() && request.words.size() != expected_words)
                impl_->reject("point selection bitset has the wrong size");
            for (const auto &p : request.polygon)
                if (!std::isfinite(p.x) || !std::isfinite(p.y))
                    impl_->reject("point selection polygon contains non-finite coordinates");
            if (request.local_clipping_planes.size() > 2)
                impl_->reject("point selection supports at most two clipping planes");
            for (const auto &plane : request.local_clipping_planes) {
                const float magnitude = std::sqrt(plane.x * plane.x + plane.y * plane.y + plane.z * plane.z);
                if (!finite(plane) || !std::isfinite(magnitude) || !(magnitude > 0))
                    impl_->reject("point selection clipping plane is non-finite or degenerate");
            }
            if (!finite(request.model_view_projection))
                impl_->reject("point selection transform is non-finite");
            impl_->resources.realize_points(request.points);
            auto words = request.words.empty() ? std::vector<uint32_t>(expected_words, 0) : request.words;
            for (size_t i = 0; i < request.points->positions().size(); ++i) {
                const vec3 &position = request.points->positions()[i];
                if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z))
                    continue;
                bool clipped = false;
                for (const auto &plane : request.local_clipping_planes)
                    clipped |= dot(plane, vec4(position, 1)) < 0;
                if (clipped)
                    continue;
                const vec4 clip = request.model_view_projection * vec4(position, 1);
                if (!(clip.w > 0))
                    continue;
                const vec3 ndc = vec3(clip.x, clip.y, clip.z) / clip.w;
                if (ndc.x < -1 || ndc.x > 1 || ndc.y < -1 || ndc.y > 1 || ndc.z < -1 || ndc.z > 1)
                    continue;
                const vec2 screen((ndc.x + 1) * 0.5f * float(request.width),
                                  (1 - ndc.y) * 0.5f * float(request.height));
                if (!inside_polygon(screen, request.polygon))
                    continue;
                const uint32_t source = request.points->source_ids().empty()
                                            ? uint32_t(i) : request.points->source_ids()[i];
                const uint32_t mask = uint32_t(1) << (source & 31);
                if (request.deselect)
                    words[source >> 5] &= ~mask;
                else
                    words[source >> 5] |= mask;
            }
            return words;
        }

        std::uint64_t OpenGLDevice::point_upload_count() const {
            return impl_->resources.uploads();
        }

        std::uint32_t OpenGLDevice::oit_dropped_fragments() const {
            return impl_->dropped_fragments;
        }

        std::unique_ptr<RenderDevice> create_device(const DeviceOptions &options) {
            if (options.validation || options.sync_validation)
                throw std::invalid_argument("OpenGL does not support requested validation diagnostics");
            if (options.profiling)
                throw std::invalid_argument("OpenGL does not support requested profiling");
            if (!options.device_name.empty())
                throw std::invalid_argument("OpenGL does not support physical-device selection");
            return std::unique_ptr<RenderDevice>(new OpenGLDevice);
        }

        std::unique_ptr<RenderDevice> create_opengl_device() {
            return create_device(DeviceOptions{});
        }

    } // namespace graphics
} // namespace easy3d
