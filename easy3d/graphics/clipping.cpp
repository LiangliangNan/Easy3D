// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/graphics/clipping.h>
#include <array>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace easy3d { namespace graphics {
std::vector<vec4> normalize_clipping_planes(const std::vector<vec4>& planes) {
    if (planes.size()>2) throw std::invalid_argument("at most two clipping planes are supported");
    std::vector<vec4> result;
    for (const auto& p:planes) {
        for (unsigned i=0;i<4;++i) if (!std::isfinite(p[i])) throw std::invalid_argument("non-finite clipping plane");
        const double length=std::sqrt(double(p.x)*p.x+double(p.y)*p.y+double(p.z)*p.z);
        if (length==0) throw std::invalid_argument("clipping plane normal must be nonzero");
        vec4 normalized;
        for (unsigned i=0;i<4;++i) {
            normalized[i]=float(p[i]/length);
            if (!std::isfinite(normalized[i])) throw std::invalid_argument("clipping plane normalization overflow");
        }
        result.push_back(normalized);
    }
    return result;
}
namespace internal {
DrawCommand clip_draw(const DrawCommand& source) {
    DrawCommand result=source;
    if (source.overlay || source.clipping_planes.empty()) return result;
    const auto& geometry=*source.geometry;
    const auto topology=geometry.topology();
    const unsigned stride=topology==Topology::triangles ? 3 : topology==Topology::lines ? 2 : 1;
    const bool analytic=(topology==Topology::points && source.point_style!=PointStyle::plain) ||
                        (topology==Topology::lines && source.line_style!=LineStyle::plain);
    // Analytic surfaces retain their proxy geometry and clip actual hits in the
    // fragment shader. The optional whole-primitive policy also tests controls.
    if (analytic && !source.discard_clipped_primitives) return result;
    const bool bake=source.lighting && !analytic && topology!=Topology::points;
    std::vector<std::array<double,4>> planes;
    for (const auto& p:source.clipping_planes) {
        std::array<double,4> local{};
        for (unsigned col=0;col<4;++col) for (unsigned row=0;row<4;++row)
            local[col]+=double(source.model_view(row,col))*p[row];
        planes.push_back(local);
    }
    auto distance=[](const Vertex& v,const std::array<double,4>& p) {
        return p[0]*v.position.x+p[1]*v.position.y+p[2]*v.position.z+p[3];
    };
    auto interpolate=[](const Vertex& a,const Vertex& b,double t) {
        Vertex v;
        for (unsigned i=0;i<3;++i) {
            v.position[i]=float((1-t)*a.position[i]+t*b.position[i]);
            v.normal[i]=float((1-t)*a.normal[i]+t*b.normal[i]);
        }
        for (unsigned i=0;i<4;++i) v.color[i]=float((1-t)*a.color[i]+t*b.color[i]);
        for (unsigned i=0;i<2;++i) v.texcoord[i]=float((1-t)*a.texcoord[i]+t*b.texcoord[i]);
        v.highlight=a.highlight; // selection is constant within each primitive
        return v;
    };
    const auto& indices=geometry.indices();
    const auto count=indices.empty() ? geometry.vertices().size() : indices.size();
    std::vector<Vertex> vertices;
    std::vector<uint32_t> ids;
    for (size_t i=0;i<count;i+=stride) {
        std::vector<Vertex> polygon;
        bool rejected=false;
        for (unsigned j=0;j<stride;++j) {
            auto v=geometry.vertices()[indices.empty() ? i+j : indices[i+j]];
            if (bake) {
                const auto n=source.normal_matrix*v.normal;
                if (dot(n,n)>0) {
                    const float diffuse=.2f+.8f*std::abs(dot(normalize(n),normalize(vec3(.3f,.5f,1))));
                    for (unsigned c=0;c<3;++c) v.color[c]*=diffuse;
                }
            }
            for (const auto& p:planes) if (distance(v,p)<0) rejected=true;
            polygon.push_back(v);
        }
        const auto id=geometry.primitive_ids().empty() ? uint32_t(i/stride) : geometry.primitive_ids()[i/stride];
        if (source.discard_clipped_primitives || stride==1) {
            if (!rejected) { vertices.insert(vertices.end(),polygon.begin(),polygon.end()); ids.push_back(id); }
            continue;
        }
        if (stride==2) {
            double lo=0,hi=1;
            for (const auto& p:planes) {
                const double a=distance(polygon[0],p),b=distance(polygon[1],p);
                if (a<0 && b<0) { lo=1; hi=0; break; }
                if (a<0) lo=std::max(lo,a/(a-b));
                if (b<0) hi=std::min(hi,a/(a-b));
            }
            if (lo<=hi) {
                vertices.push_back(interpolate(polygon[0],polygon[1],lo));
                vertices.push_back(interpolate(polygon[0],polygon[1],hi)); ids.push_back(id);
            }
        } else {
            for (const auto& p:planes) {
                if (polygon.empty()) break;
                std::vector<Vertex> clipped;
                auto a=polygon.back(); auto da=distance(a,p);
                for (const auto& b:polygon) {
                    const double db=distance(b,p);
                    if ((da<0)!=(db<0)) clipped.push_back(interpolate(a,b,da/(da-db)));
                    if (db>=0) clipped.push_back(b);
                    a=b; da=db;
                }
                polygon=std::move(clipped);
            }
            for (size_t j=1;j+1<polygon.size();++j) {
                vertices.push_back(polygon[0]); vertices.push_back(polygon[j]); vertices.push_back(polygon[j+1]); ids.push_back(id);
            }
        }
    }
    result.geometry=std::make_shared<Geometry>(std::move(vertices),std::vector<uint32_t>{},topology,std::move(ids));
    if (bake) result.lighting=false; // interpolate original vertex lighting, not newly normalized cut normals
    return result;
}
}
} }
