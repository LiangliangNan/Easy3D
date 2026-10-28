// SPDX-License-Identifier: GPL-3.0-or-later
#include "shape.h"
#include <easy3d/renderer/shape.h>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace easy3d { namespace vulkan { namespace shape {
namespace {
void quad(std::vector<graphics::Vertex>& vertices, const vec2& a, const vec2& b,
          const vec2& c, const vec2& d) {
    for (const auto& p : {a,b,c,a,c,d}) {
        graphics::Vertex v; v.position=vec3(p.x,p.y,0); vertices.push_back(v);
    }
}
graphics::DrawCommand command(std::vector<graphics::Vertex> vertices, const vec4& color,
                              unsigned width, unsigned height) {
    graphics::DrawCommand result;
    result.geometry=std::make_shared<graphics::Geometry>(std::move(vertices));
    result.color=color;
    result.overlay=true;
    result.model_view_projection=graphics::screen_space_projection(width,height);
    return result;
}
}
graphics::DrawCommand draw_quad_filled(const Rect& rect,const vec4& color,unsigned width,unsigned height) {
    std::vector<graphics::Vertex> vertices;
    quad(vertices,vec2(rect.x_min(),rect.y_min()),vec2(rect.x_max(),rect.y_min()),
                  vec2(rect.x_max(),rect.y_max()),vec2(rect.x_min(),rect.y_max()));
    return command(std::move(vertices),color,width,height);
}
graphics::DrawCommand draw_lines_2d(const std::vector<vec2>& segments,const vec4& color,
                                   unsigned width,unsigned height,float line_width) {
    if (segments.size()%2 || !std::isfinite(line_width) || line_width<=0)
        throw std::invalid_argument("screen lines require pairs of endpoints and a positive finite width");
    std::vector<graphics::Vertex> vertices;
    for (size_t i=0;i<segments.size();i+=2) {
        const auto delta=segments[i+1]-segments[i];
        const float length=delta.norm();
        if (!std::isfinite(length)) throw std::invalid_argument("non-finite screen line");
        if (!length) continue;
        const vec2 offset=vec2(-delta.y,delta.x)*(line_width*.5f/length);
        quad(vertices,segments[i]-offset,segments[i+1]-offset,segments[i+1]+offset,segments[i]+offset);
    }
    return command(std::move(vertices),color,width,height);
}
graphics::DrawCommand draw_quad_wire(const Rect& r,const vec4& color,unsigned width,unsigned height,float line_width) {
    const vec2 a(r.x_min(),r.y_min()),b(r.x_max(),r.y_min()),c(r.x_max(),r.y_max()),d(r.x_min(),r.y_max());
    return draw_lines_2d({a,b,b,c,c,d,d,a},color,width,height,line_width);
}
graphics::DrawCommand draw_box_wire(const mat4& mvp,unsigned width,unsigned height,float line_width,bool abstracted) {
    if (!width || !height || !std::isfinite(line_width) || line_width<=0)
        throw std::invalid_argument("box guide requires a positive viewport and finite line width");
    std::vector<vec3> points,colors;
    easy3d::shape::create_box(points,colors,abstracted);
    std::vector<graphics::Vertex> vertices;
    for (size_t i=0;i<points.size();i+=2) {
        vec4 a=mvp*vec4(points[i],1),b=mvp*vec4(points[i+1],1);
        for (unsigned c=0;c<4;++c) if (!std::isfinite(a[c]) || !std::isfinite(b[c]))
            throw std::invalid_argument("non-finite box guide transform");
        bool visible=true;
        // Clip before dividing by w, including segments crossing the near/eye
        // planes. Ribbons keep linearly interpolated screen-space surface depth.
        for (unsigned axis=0;axis<3 && visible;++axis) for (float sign:{-1.0f,1.0f}) {
            const float da=a.w+sign*a[axis],db=b.w+sign*b[axis];
            if (da<0 && db<0) {visible=false;break;}
            if ((da<0)!=(db<0)) {
                const auto p=a+(b-a)*(da/(da-db));
                if (da<0) a=p;else b=p;
            }
        }
        if (!visible || a.w<=1e-8f || b.w<=1e-8f) continue;
        const vec3 p=a.xyz()/a.w,q=b.xyz()/b.w;
        const vec2 delta((q.x-p.x)*width*.5f,(q.y-p.y)*height*.5f);
        const float length=delta.norm();
        if (length<=1e-6f) continue;
        const vec2 offset=vec2(-delta.y,delta.x)*(line_width*.5f/length);
        const vec3 d(2*offset.x/width,2*offset.y/height,0);
        for (const auto& point:{p-d,q-d,q+d,p-d,q+d,p+d}) {
            graphics::Vertex v;v.position=point;
            // Keep a coplanar guide visible despite projection/depth rounding.
            // This only offsets the annotation, never scene depth or picking.
            v.position.z=std::max(-1.0f,v.position.z-4*std::numeric_limits<float>::epsilon());
            v.color=vec4(colors[i],1);vertices.push_back(v);
        }
    }
    graphics::DrawCommand result;
    result.geometry=std::make_shared<graphics::Geometry>(std::move(vertices));
    result.overlay=result.overlay_depth_test=true;
    return result;
}
} } }
