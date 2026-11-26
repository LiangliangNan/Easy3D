// SPDX-License-Identifier: GPL-3.0-or-later
#include "render_device.h"

#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/backends/opengl/overlay_renderer.h>
#include <easy3d/renderer/backends/opengl/query_state.h>

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
uniform bool lighting, material_lighting, two_sides, distinct_back, ignore_highlight;
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
        if (!material_lighting) {
            out_color.rgb*=0.2+0.8*abs(dot(n,normalize(vec3(0.3,0.5,1))));
        } else {
            vec3 view_dir=normalize(eye-world_position), l=normalize(light);
            if (distinct_back && !gl_FrontFacing) out_color.rgb=back_color.rgb;
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
uniform vec3 eye, light, sphere_light;
uniform vec2 depth_range;
uniform vec4 ambient, specular;
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
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, data->width(), data->height(), 0,
                                 GL_RGBA, GL_UNSIGNED_BYTE, data->rgba().data());
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
            ResourceCache resources;
            FrameTarget target;
            opengl::OverlayRenderer overlays;

            ~Impl() {
                if (point_program)
                    glDeleteProgram(point_program);
                if (line_program)
                    glDeleteProgram(line_program);
                if (wide_line_program)
                    glDeleteProgram(wide_line_program);
                if (program)
                    glDeleteProgram(program);
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
                if (transparency != TransparencyMethod::sorted)
                    reject("this OpenGL RenderDevice increment supports sorted transparency only");
                if (!(capabilities() & samples))
                    reject("unsupported OpenGL MSAA sample count");
            }

            void validate(const DrawCommand &d, uint32_t width, uint32_t height) {
                if (!d.geometry)
                    reject("draw command has no geometry");
                viewport_of(d, width, height);
                if (!std::isfinite(d.depth_range.x) || !std::isfinite(d.depth_range.y) ||
                    d.depth_range.x < 0 || d.depth_range.x > d.depth_range.y || d.depth_range.y > 1)
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
                if (d.color.w < 0 || d.color.w > 1)
                    reject("draw alpha must be in [0,1]");
                if (d.overlay_depth_test && !d.overlay)
                    reject("scene-depth annotation requires overlay=true");
                if (d.overlay_independent_depth && (!d.overlay || d.overlay_depth_test))
                    reject("independent-depth HUD requires overlay=true and excludes scene-depth testing");
                if (d.scene_depth_texture || d.eye_dome_lighting || d.ambient_occlusion || d.shadow)
                    reject("full-screen depth/effect overlays are not supported by this OpenGL RenderDevice increment");
                if (d.shadow_caster || d.shadow_background)
                    reject("shadow commands are not supported by this OpenGL RenderDevice increment");
                if (!d.clipping_planes.empty() || !d.object_clipping_planes.empty() || d.discard_clipped_primitives)
                    reject("clipping is not supported by this OpenGL RenderDevice increment");
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
                if (!d.overlay && d.object_id == UINT32_MAX)
                    reject("UINT32_MAX is reserved for the background object ID");
            }

            unsigned capabilities() {
                if (sample_mask_ready)
                    return sample_mask;
                opengl::QueryState state;
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
                std::vector<uint32_t> ids;
                if (!d.geometry->primitive_ids().empty())
                    ids.push_back(d.geometry->primitive_ids()[triangle]);
                DrawCommand expanded = d;
                expanded.geometry = std::make_shared<Geometry>(std::move(vertices), std::vector<uint32_t>{0, 1, 2},
                                                              Topology::triangles, std::move(ids));
                return expanded;
            }

            std::vector<DrawCommand> prepare(const std::vector<DrawCommand> &commands, uint32_t width, uint32_t height) {
                std::vector<DrawCommand> opaque, transparent;
                for (const auto &d : commands) {
                    if (d.overlay)
                        continue;
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
                    return clip.w > 0 ? clip.z / clip.w : std::numeric_limits<float>::infinity();
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
                glUniform4fv(location(p == program ? "color" : "tint"), 1, d.color.data());
                glUniform3fv(location("eye"), 1, d.eye.data());
                glUniform3fv(location("light"), 1, d.light.data());
                glUniform4fv(location("ambient"), 1, d.ambient.data());
                glUniform4fv(location("specular"), 1, d.specular.data());
                glUniform4fv(location("back_color"), 1, d.back_color.data());
                glUniform1f(location("shininess"), d.shininess);
                glUniform1i(location("lighting"), d.lighting);
                glUniform1i(location("material_lighting"), d.material_lighting);
                glUniform1i(location("two_sides"), d.lighting_two_sides);
                glUniform1i(location("distinct_back"), d.distinct_back_color);
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
                glUniform2fv(location("depth_range"), 1, d.depth_range.data());
            }

            void draw(const DrawCommand &d, uint32_t width, uint32_t height, bool color_pass,
                      bool transparent, unsigned samples) {
                const ivec4 viewport = viewport_of(d, width, height);
                const int x = viewport.x, y = int(height) - viewport.y - viewport.w;
                glViewport(x, y, viewport.z, viewport.w);
                glDepthRange(d.depth_range.x, d.depth_range.y);
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
                    glPolygonOffset(1, 1);
                } else {
                    glDisable(GL_POLYGON_OFFSET_FILL);
                }
                const bool points = d.geometry->topology() == Topology::points;
                const bool lines = d.geometry->topology() == Topology::lines;
                const bool analytic = points ? d.point_style != PointStyle::plain
                                             : lines && (d.line_style != LineStyle::plain ||
                                                         d.line_width != 1 || !d.line_screen_space_width);
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

            void draw_overlays(const std::vector<DrawCommand> &commands, uint32_t width, uint32_t height) {
                std::vector<DrawCommand> overlays;
                for (const auto &d : commands) {
                    if (d.overlay)
                        overlays.push_back(d);
                }
                auto &output = target.color();
                output.bind();
                output.activate_draw_buffer(0);
                this->overlays.set(std::move(overlays), false);
                this->overlays.draw(false, int(width), int(height), ivec4(0), {});
            }

            void render(uint32_t width, uint32_t height, const std::vector<DrawCommand> &commands,
                        const vec4 &background, unsigned samples, FrameReadback readback,
                        bool overlays, bool include_color) {
                require_context();
                resources.prune();
                validate(width, height, samples, TransparencyMethod::sorted);
                for (const auto &d : commands)
                    validate(d, width, height);
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
                draw_scene(scene, width, height, include_color, true, samples);
                if (include_color) {
                    glViewport(0, 0, int(width), int(height));
                    glScissor(0, 0, int(width), int(height));
                    if (overlays)
                        draw_overlays(commands, width, height);
                    glDisable(GL_SCISSOR_TEST);
                    auto &color = target.color();
                    auto &output = target.output();
                    color.bind(GL_READ_FRAMEBUFFER);
                    output.bind(GL_DRAW_FRAMEBUFFER);
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, color.handle());
                    glReadBuffer(GL_COLOR_ATTACHMENT0);
                    glBlitFramebuffer(0, 0, int(width), int(height), 0, 0, int(width), int(height),
                                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
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

            void check_error() {
                const GLenum error = glGetError();
                if (error != GL_NO_ERROR)
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
                                        unsigned, FrameReadback readback) {
            if (transparency != TransparencyMethod::sorted)
                impl_->reject("this OpenGL RenderDevice increment supports sorted transparency only");
            if (readback != FrameReadback::color && readback != FrameReadback::color_depth_ids)
                impl_->reject("unknown frame readback mode");
            opengl::QueryState state;
            state.prepare(int(width), int(height));
            impl_->render(width, height, commands, background, samples, readback, true, true);
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
            impl_->render(width, height, commands, vec4(0), 1, FrameReadback::color_depth_ids, false, false);
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
            impl_->render(width, height, commands, vec4(0), 1, FrameReadback::color_depth_ids, false, false);
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
                                   unsigned) {
            if (transparency != TransparencyMethod::sorted)
                impl_->reject("this OpenGL RenderDevice increment supports sorted transparency only");
            opengl::QueryState state;
            state.prepare(int(width), int(height));
            const auto check = [&](const char *stage) {
                const GLenum error = glGetError();
                if (error != GL_NO_ERROR)
                    throw std::runtime_error(std::string("OpenGL presentation failed at ") + stage + ": " +
                                             std::to_string(error));
            };
            impl_->render(width, height, commands, background, samples, FrameReadback::color, true, true);
            check("scene rendering");
            auto &output = impl_->target.output();
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GLenum(state.draw_fbo));
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
            check("destination query");
            // Keep sample coverage intact when the caller already has a compatible
            // target; resolving first would decorrelate later MSAA HUD composition.
            const bool compatible = destination_samples == int(samples > 1 ? samples : 0);
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
                auto &depth = compatible && samples > 1 ? impl_->target.color() : impl_->target.scene();
                depth.bind(GL_READ_FRAMEBUFFER);
                glReadBuffer(GL_COLOR_ATTACHMENT0);
                glBlitFramebuffer(0, 0, int(width), int(height), 0, 0, int(width), int(height),
                                  GL_DEPTH_BUFFER_BIT, GL_NEAREST);
                // Core-profile drivers need not expose default-framebuffer
                // attachment queries; a window may also have no depth buffer.
                const GLenum depth_error = glGetError();
                if (depth_error != GL_NO_ERROR && depth_error != GL_INVALID_OPERATION)
                    throw std::runtime_error(std::string("OpenGL presentation failed at default depth blit: ") +
                                             std::to_string(depth_error));
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
                const bool multisample_depth = compatible && samples > 1;
                auto &depth = multisample_depth ? impl_->target.color() : impl_->target.scene();
                const GLenum destination_format = depth_format(GLuint(state.draw_fbo), unsigned(destination_samples));
                const GLenum source_format = depth_format(depth.handle(), multisample_depth ? samples : 0);
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
            return 0;
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
