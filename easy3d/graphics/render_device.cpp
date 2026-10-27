// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/graphics/render_device.h>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace easy3d { namespace graphics {
Geometry::Geometry(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices,
                   Topology topology, std::vector<std::uint32_t> ids)
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
            if (!std::isfinite(v.color[i])) throw std::invalid_argument("non-finite color");
        for (int i = 0; i < 2; ++i)
            if (!std::isfinite(v.texcoord[i])) throw std::invalid_argument("non-finite texture coordinate");
    }
    for (auto id : primitive_ids_)
        if (id == UINT32_MAX) throw std::invalid_argument("UINT32_MAX is reserved for background");
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
