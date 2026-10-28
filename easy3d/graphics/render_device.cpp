// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/graphics/render_device.h>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace easy3d { namespace graphics {
PointData::PointData(std::vector<vec3> positions,std::vector<uint32_t> ids,uint32_t count)
    :positions_(std::move(positions)),source_ids_(std::move(ids)),source_count_(count) {
    if (positions_.size()>UINT32_MAX) throw std::invalid_argument("too many point positions");
    if (source_ids_.empty()) {
        source_ids_.resize(positions_.size());
        for (size_t i=0;i<positions_.size();++i) source_ids_[i]=uint32_t(i);
    }
    if (source_ids_.size()!=positions_.size()) throw std::invalid_argument("one source ID is required per point");
    if (!source_count_) source_count_=uint32_t(positions_.size());
    for (auto id:source_ids_) if (id>=source_count_) throw std::invalid_argument("point source ID outside source slots");
}
Geometry::Geometry(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices,
                   Topology topology, std::vector<std::uint32_t> ids,std::shared_ptr<const PointData> points)
    : vertices_(std::move(vertices)), indices_(std::move(indices)), topology_(topology),
      primitive_ids_(std::move(ids)) {
    if (topology != Topology::triangles && topology != Topology::lines && topology != Topology::points)
        throw std::invalid_argument("unknown primitive topology");
    const auto count = indices_.empty() ? vertices_.size() : indices_.size();
    const unsigned size = topology == Topology::triangles ? 3 : topology == Topology::lines ? 2 : 1;
    if (count % size || count > std::numeric_limits<std::uint32_t>::max())
        throw std::invalid_argument("invalid primitive element count");
    if (!primitive_ids_.empty() && primitive_ids_.size() != count / size)
        throw std::invalid_argument("one ID is required per primitive");
    for (auto index : indices_)
        if (index >= vertices_.size()) throw std::out_of_range("geometry index out of range");
    for (const auto& v : vertices_) {
        for (int i = 0; i < 3; ++i)
            if (!std::isfinite(v.position[i]) || !std::isfinite(v.normal[i]))
                throw std::invalid_argument("non-finite vertex");
        for (int i = 0; i < 4; ++i)
            if (!std::isfinite(v.color[i]) || !std::isfinite(v.highlight[i])) throw std::invalid_argument("non-finite color/highlight");
        if (v.highlight.w<0 || v.highlight.w>1) throw std::invalid_argument("highlight strength must be in [0,1]");
        for (int i = 0; i < 2; ++i)
            if (!std::isfinite(v.texcoord[i])) throw std::invalid_argument("non-finite texture coordinate");
    }
    for (auto id : primitive_ids_)
        if (id == UINT32_MAX) throw std::invalid_argument("UINT32_MAX is reserved for background");
    if (topology_==Topology::points) {
        if (points) {
            if (points->positions().size()!=count) throw std::invalid_argument("point resource count does not match geometry");
            for (size_t i=0;i<count;++i) {
                const auto index=indices_.empty()?uint32_t(i):indices_[i];
                if (points->positions()[i]!=vertices_[index].position)
                    throw std::invalid_argument("point resource does not match point geometry");
            }
            point_data_=std::move(points);
        } else {
            if (vertices_.size()>UINT32_MAX) throw std::invalid_argument("too many point source slots");
            std::vector<vec3> positions; std::vector<uint32_t> sources;
            positions.reserve(count); sources.reserve(count);
            for (size_t i=0;i<count;++i) {
                const auto index=indices_.empty()?uint32_t(i):indices_[i];
                positions.push_back(vertices_[index].position); sources.push_back(index);
            }
            point_data_=std::make_shared<PointData>(std::move(positions),std::move(sources),uint32_t(vertices_.size()));
        }
    } else if (points) throw std::invalid_argument("point resources require point geometry");
}
mat4 screen_space_projection(std::uint32_t width, std::uint32_t height) {
    if (!width || !height) throw std::invalid_argument("screen-space projection requires positive dimensions");
    mat4 result = mat4::identity();
    result(0,0) = 2.0f/width; result(1,1) = -2.0f/height;
    result(0,3) = -1; result(1,3) = 1;
    return result;
}
Image::Image(std::uint32_t width, std::uint32_t height, std::vector<unsigned char> rgba, Sampler sampler)
    : width_(width), height_(height), rgba_(std::move(rgba)), sampler_(sampler) {
    if ((sampler.wrap != TextureWrap::clamp_to_edge && sampler.wrap != TextureWrap::repeat) ||
        (sampler.filter != TextureFilter::nearest && sampler.filter != TextureFilter::linear))
        throw std::invalid_argument("invalid texture sampler");
    if (!width || !height || std::uint64_t(width) * height > std::numeric_limits<std::size_t>::max() / 4 ||
        rgba_.size() != std::uint64_t(width) * height * 4)
        throw std::invalid_argument("invalid RGBA8 image dimensions or storage");
}
} }
