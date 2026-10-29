// SPDX-License-Identifier: GPL-3.0-or-later
#include "drawable.h"
#include "shape.h"
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/util/setting.h>
#include <cmath>
#include <limits>
#include <map>
#include <stdexcept>

namespace easy3d { namespace vulkan {
struct DrawableRenderer::Impl {
    struct Cached {
        std::weak_ptr<Drawable> source;
        std::size_t revision = 0;
        bool use_colors = false;
        bool flat_shading = false;
        bool textured = false;
        bool selected = false, highlighted = false;
        std::pair<int,int> highlight_range = {-1,-1};
        vec4 highlight_color = vec4(0);
        float repeat = 1;
        std::shared_ptr<const graphics::Geometry> geometry;
        std::shared_ptr<const graphics::PointData> points;
        std::shared_ptr<const graphics::Image> texture_image;
    };
    mutable std::map<const Drawable*, Cached> cache;
    struct CachedScreenLines {
        std::weak_ptr<Drawable> source;
        std::size_t revision = 0;
        int width = 0, height = 0;
        float line_width = 0;
        graphics::DrawCommand command;
    };
    mutable std::map<const Drawable*, CachedScreenLines> screen_lines;

    graphics::DrawCommand screen_command(const std::shared_ptr<Drawable>& drawable,const Camera& camera) const {
        auto& d=*drawable;
        const auto width=static_cast<LinesDrawable2D&>(d).line_width();
        if (!d.has_cpu_buffers()) throw std::logic_error("Vulkan screen-space lines require CPU buffers");
        if (d.coloring_method()!=State::UNIFORM_COLOR)
            throw std::logic_error("screen-space lines support uniform coloring only");
        const auto& data=d.cpu_buffers();
        auto& entry=screen_lines[&d];
        if (entry.source.lock()!=drawable || !entry.command.geometry || entry.revision!=data.revision ||
            entry.width!=camera.screenWidth() || entry.height!=camera.screenHeight() || entry.line_width!=width) {
            const size_t count=data.indices.empty()?data.vertices.size():data.indices.size();
            if (count%2) throw std::invalid_argument("screen-space lines require endpoint pairs");
            std::vector<vec2> segments;segments.reserve(count);
            for (size_t i=0;i<count;++i) {
                const size_t index=data.indices.empty()?i:data.indices[i];
                if (index>=data.vertices.size()) throw std::out_of_range("screen-space line index out of range");
                const auto& p=data.vertices[index];
                segments.emplace_back((p.x+1)*camera.screenWidth()*.5f,(1-p.y)*camera.screenHeight()*.5f);
            }
            entry.command=shape::draw_lines_2d(segments,vec4(1),camera.screenWidth(),camera.screenHeight(),width);
            entry.source=drawable;entry.revision=data.revision;entry.width=camera.screenWidth();entry.height=camera.screenHeight();
            entry.line_width=width;
        }
        auto result=entry.command;result.color=d.color();return result;
    }

    struct CachedImage {
        std::weak_ptr<const Texture::PixelData> source;
        std::weak_ptr<const graphics::Image> image;
    };
    mutable std::map<const Texture::PixelData*, CachedImage> images;
    std::shared_ptr<const graphics::Image> image(const Texture* texture) const {
        if (!texture || !texture->pixels()) throw std::logic_error("texture coloring requires CPU image data");
        const auto& pixels = texture->pixels();
        auto& cached = images[pixels.get()];
        if (cached.source.lock() == pixels) {
            auto existing = cached.image.lock();
            if (existing) return existing;
        }
        const uint64_t count = uint64_t(pixels->width)*pixels->height;
        if (count > std::numeric_limits<size_t>::max()/4) throw std::invalid_argument("texture is too large for RGBA8");
        std::vector<unsigned char> rgba(size_t(count)*4);
        for (int y = 0; y < pixels->height; ++y) for (int x = 0; x < pixels->width; ++x) {
            const size_t source = (size_t(pixels->height-1-y)*pixels->width+x)*pixels->channels;
            const size_t target = (size_t(y)*pixels->width+x)*4;
            rgba[target] = pixels->bytes[source];
            rgba[target+1] = pixels->channels > 1 ? pixels->bytes[source+1] : 0;
            rgba[target+2] = pixels->channels > 2 ? pixels->bytes[source+2] : 0;
            rgba[target+3] = pixels->channels > 3 ? pixels->bytes[source+3] : 255;
        }
        graphics::Sampler sampler;
        sampler.wrap = texture->wrap_mode() == Texture::REPEAT ? graphics::TextureWrap::repeat : graphics::TextureWrap::clamp_to_edge;
        sampler.filter = texture->filter_mode() == Texture::LINEAR ? graphics::TextureFilter::linear : graphics::TextureFilter::nearest;
        auto result = std::make_shared<graphics::Image>(pixels->width,pixels->height,std::move(rgba),sampler);
        cached.source = pixels; cached.image = result;
        return result;
    }

    graphics::DrawCommand command(const std::shared_ptr<Drawable>& drawable, uint32_t id, const Camera& cam, const std::vector<vec4>& clipping_planes, bool model_selected) const {
        auto& d = *drawable;
        if (dynamic_cast<LinesDrawable2D*>(&d)) return screen_command(drawable,cam);
        if (!d.has_cpu_buffers())
            throw std::logic_error("drawable must use CPU buffers before Vulkan rendering");
        if (d.coloring_method() != State::UNIFORM_COLOR && d.coloring_method() != State::COLOR_PROPERTY &&
            d.coloring_method() != State::TEXTURED && d.coloring_method() != State::SCALAR_FIELD)
            throw std::logic_error("unknown drawable coloring method");
        if (d.color().w != 1 || d.is_ssao_enabled())
            throw std::logic_error("opacity and SSAO are not yet ported");
        if (auto* p = dynamic_cast<PointsDrawable*>(&d)) {
            if (p->impostor_type() != PointsDrawable::PLAIN && p->impostor_type() != PointsDrawable::SPHERE &&
                p->impostor_type() != PointsDrawable::SURFEL)
                throw std::logic_error("unknown point impostor type");
        }
        if (auto* l = dynamic_cast<LinesDrawable*>(&d)) {
            if (l->impostor_type() != LinesDrawable::PLAIN && l->impostor_type() != LinesDrawable::CYLINDER &&
                l->impostor_type() != LinesDrawable::CONE)
                throw std::logic_error("unknown line impostor type");
            if (!std::isfinite(l->line_width()) || l->line_width() <= 0)
                throw std::invalid_argument("line width must be finite and positive");
        }
        // As with the legacy opaque pass, TrianglesDrawable::opacity() is only
        // meaningful in a separate transparency pass (not implemented here).
        const auto& buffers = d.cpu_buffers();
        if (auto* p = dynamic_cast<PointsDrawable*>(&d))
            if (p->impostor_type() == PointsDrawable::SURFEL && !buffers.vertices.empty() && buffers.normals.empty())
                throw std::logic_error("surfel rendering requires point normals");
        auto& entry = cache[&d];
        const bool use_colors = d.coloring_method() == State::COLOR_PROPERTY;
        const bool textured = d.coloring_method() == State::TEXTURED || d.coloring_method() == State::SCALAR_FIELD;
        const float repeat = d.texture_repeat()+d.texture_fractional_repeat()/100.0f;
        if (textured && !std::isfinite(repeat)) throw std::invalid_argument("texture repeat must be finite");
        const auto* triangles = dynamic_cast<const TrianglesDrawable*>(&d);
        const bool flat_shading = triangles && !triangles->smooth_shading();
        const bool selected = model_selected || d.is_selected();
        const bool highlighted = d.highlight();
        if (entry.source.lock() != drawable || entry.revision != buffers.revision || !entry.geometry ||
            entry.use_colors != use_colors || entry.flat_shading != flat_shading ||
            entry.textured != textured || (textured && entry.repeat != repeat) ||
            entry.selected != selected || entry.highlighted != highlighted ||
            entry.highlight_range != d.highlight_range() || entry.highlight_color != setting::highlight_color) {
            const auto n = buffers.vertices.size();
            if ((!buffers.colors.empty() && buffers.colors.size() != n) ||
                (!buffers.normals.empty() && buffers.normals.size() != n))
                throw std::logic_error("drawable attribute count does not match vertices");
            if (textured && buffers.texcoords.size() != n)
                throw std::logic_error("texture coordinate count does not match vertices");
            std::vector<graphics::Vertex> vertices(n);
            for (size_t i = 0; i < n; ++i) {
                vertices[i].position = buffers.vertices[i];
                if (!buffers.normals.empty()) vertices[i].normal = buffers.normals[i];
                if (use_colors && !buffers.colors.empty()) vertices[i].color = vec4(buffers.colors[i], 1);
                if (textured) vertices[i].texcoord = vec2(buffers.texcoords[i].x*repeat,1-buffers.texcoords[i].y*repeat);
            }
            const auto topology = d.type() == Drawable::DT_TRIANGLES ? graphics::Topology::triangles :
                                  d.type() == Drawable::DT_LINES ? graphics::Topology::lines : graphics::Topology::points;
            if (topology==graphics::Topology::points) {
                const auto count=buffers.indices.empty()?n:buffers.indices.size();
                if (n>UINT32_MAX) throw std::invalid_argument("too many point source slots");
                bool unchanged=entry.points && entry.points->positions().size()==count && entry.points->source_count()==n;
                for (size_t i=0;i<count;++i) {
                    const auto index=buffers.indices.empty()?uint32_t(i):buffers.indices[i];
                    if (index>=n) throw std::out_of_range("point drawable index out of range");
                    if (unchanged && (entry.points->positions()[i]!=buffers.vertices[index] || entry.points->source_ids()[i]!=index))
                        unchanged=false;
                }
                if (!unchanged) {
                    std::vector<vec3> positions; std::vector<uint32_t> ids;
                    positions.reserve(count); ids.reserve(count);
                    for (size_t i=0;i<count;++i) {
                        const auto index=buffers.indices.empty()?uint32_t(i):buffers.indices[i];
                        positions.push_back(buffers.vertices[index]); ids.push_back(index);
                    }
                    entry.points=std::make_shared<graphics::PointData>(std::move(positions),std::move(ids),uint32_t(n));
                }
            } else entry.points.reset();
            auto geometry = std::make_shared<graphics::Geometry>(vertices, buffers.indices, topology,std::vector<uint32_t>{},entry.points);
            if (flat_shading) {
                // Geometry has already validated the indices and triangle count.
                // Give every corner its triangle normal while preserving colors
                // and primitive order, including indexed/shared-vertex inputs.
                const auto& indices = geometry->indices();
                const size_t count = indices.empty() ? vertices.size() : indices.size();
                std::vector<graphics::Vertex> flat(count);
                for (size_t i = 0; i < count; i += 3) {
                    for (size_t j = 0; j < 3; ++j)
                        flat[i+j] = vertices[indices.empty() ? i+j : indices[i+j]];
                    const vec3 normal = cross(flat[i+1].position-flat[i].position,
                                              flat[i+2].position-flat[i].position);
                    for (size_t j = 0; j < 3; ++j) flat[i+j].normal = normal;
                }
                geometry = std::make_shared<graphics::Geometry>(std::move(flat), std::vector<uint32_t>{}, topology);
            }
            if (selected || highlighted) {
                const auto& source=geometry->vertices(); const auto& indices=geometry->indices();
                const auto count=indices.empty() ? source.size() : indices.size();
                const unsigned stride=topology==graphics::Topology::triangles ? 3 : topology==graphics::Topology::lines ? 2 : 1;
                std::vector<graphics::Vertex> marked(count);
                const auto range=d.highlight_range();
                for (size_t i=0;i<count;++i) {
                    marked[i]=source[indices.empty() ? i : indices[i]];
                    float strength=selected ? 0.6f : 0;
                    if (highlighted && range.first>=0 && i/stride>=size_t(range.first) &&
                        range.second>=range.first && i/stride<=size_t(range.second)) strength=0.8f+0.2f*strength;
                    marked[i].highlight=vec4(setting::highlight_color.x,setting::highlight_color.y,setting::highlight_color.z,strength);
                }
                geometry=std::make_shared<graphics::Geometry>(std::move(marked),std::vector<uint32_t>{},topology,std::vector<uint32_t>{},entry.points);
            }
            entry.selected=selected; entry.highlighted=highlighted;
            entry.highlight_range=d.highlight_range(); entry.highlight_color=setting::highlight_color;
            entry.source = drawable; entry.revision = buffers.revision; entry.geometry = geometry;
            entry.use_colors = use_colors; entry.flat_shading = flat_shading;
            entry.textured = textured; entry.repeat = repeat;
        }
        graphics::DrawCommand result;
        result.geometry = entry.geometry;
        entry.texture_image = textured ? image(d.texture()) : nullptr;
        result.texture = entry.texture_image;
        if (auto* p = dynamic_cast<PointsDrawable*>(&d)) {
            result.point_size = p->point_size();
            result.point_style = p->impostor_type() == PointsDrawable::SPHERE ? graphics::PointStyle::sphere :
                                 p->impostor_type() == PointsDrawable::SURFEL ? graphics::PointStyle::surfel : graphics::PointStyle::plain;
            result.sphere_radius = p->point_size() * cam.pixelGLRatio(cam.pivotPoint()) * 0.5f;
            result.model_view = cam.modelViewMatrix() * d.manipulated_matrix();
            result.projection = cam.projectionMatrix();
            result.sphere_light = mat3(cam.modelViewMatrix()) * vec3(0.3f, 0.5f, 1);
        }
        if (auto* l = dynamic_cast<LinesDrawable*>(&d)) {
            result.line_style = l->impostor_type() == LinesDrawable::CYLINDER ? graphics::LineStyle::cylinder :
                                l->impostor_type() == LinesDrawable::CONE ? graphics::LineStyle::cone : graphics::LineStyle::plain;
            result.line_width = l->line_width();
            result.line_radius = l->line_width() * cam.pixelGLRatio(cam.pivotPoint()) * 0.5f;
            result.model_view = cam.modelViewMatrix() * d.manipulated_matrix();
            result.projection = cam.projectionMatrix();
            result.sphere_light = mat3(cam.modelViewMatrix()) * vec3(0.3f, 0.5f, 1);
        }
        result.model_view_projection = cam.modelViewProjectionMatrix() * d.manipulated_matrix();
        if (!clipping_planes.empty()) {
            result.model_view=cam.modelViewMatrix()*d.manipulated_matrix();
            const auto plane_transform=transpose(inverse(cam.modelViewMatrix()));
            for (const auto& plane:clipping_planes) result.clipping_planes.push_back(plane_transform*plane);
            result.discard_clipped_primitives=d.plane_clip_discard_primitive();
        }
        if (d.lighting()) result.normal_matrix = transform::normal_matrix(d.manipulated_matrix());
        result.color = d.coloring_method() == State::UNIFORM_COLOR ? d.color() : vec4(1, 1, 1, 1);
        result.object_id = id;
        result.lighting = d.lighting();
        if (dynamic_cast<PointsDrawable*>(&d) || dynamic_cast<TrianglesDrawable*>(&d)) {
            result.material_lighting = true;
            result.lighting_two_sides = d.lighting_two_sides();
            result.distinct_back_color = d.distinct_back_color();
            result.back_color = d.back_color();
            result.ambient = d.material().ambient;
            result.specular = d.material().specular;
            result.shininess = d.material().shininess;
            result.model = d.manipulated_matrix();
            result.eye = cam.position();
            result.light = vec3(inverse(cam.modelViewMatrix()) * setting::light_position);
            if (auto* p = dynamic_cast<PointsDrawable*>(&d)) {
                if (p->impostor_type() != PointsDrawable::PLAIN) {
                    result.eye = vec3(0);
                    result.light = vec3(setting::light_position);
                }
                if (p->impostor_type() != PointsDrawable::SPHERE && d.cpu_buffers().normals.empty())
                    result.lighting = false;
            }
        }
        return result;
    }
    void prune() const {
        for (auto it=screen_lines.begin();it!=screen_lines.end();) {
            if (it->second.source.expired()) it=screen_lines.erase(it); else ++it;
        }
        for (auto it = cache.begin(); it != cache.end(); ) {
            if (it->second.source.expired()) it = cache.erase(it); else ++it;
        }
        for (auto it = images.begin(); it != images.end(); ) {
            if (it->second.source.expired() || it->second.image.expired()) it = images.erase(it); else ++it;
        }
    }
};

DrawableRenderer::DrawableRenderer() : impl_(new Impl) {}
DrawableRenderer::~DrawableRenderer() = default;
graphics::DrawCommand DrawableRenderer::command(const std::shared_ptr<Drawable>& drawable,
        uint32_t id, const Camera& camera, const std::vector<vec4>& clipping_planes,
        bool model_selected) const {
    return impl_->command(drawable, id, camera, clipping_planes, model_selected);
}
void DrawableRenderer::prune() const { impl_->prune(); }
void DrawableRenderer::clear() { impl_->cache.clear(); impl_->screen_lines.clear(); }
} } // namespace easy3d::vulkan
