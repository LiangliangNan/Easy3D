// SPDX-License-Identifier: GPL-3.0-or-later
#include "shape.h"
#include <cmath>
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
} } }
