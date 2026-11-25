// SPDX-License-Identifier: GPL-3.0-or-later
#include "render_device.h"

#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl.h>
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

            GLuint compile_shader(GLenum type, const char *source) {
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

            GLuint create_program() {
                const GLuint vertex = compile_shader(GL_VERTEX_SHADER, vertex_shader);
                GLuint fragment = 0;
                try {
                    fragment = compile_shader(GL_FRAGMENT_SHADER, fragment_shader);
                } catch (...) {
                    glDeleteShader(vertex);
                    throw;
                }
                const GLuint program = glCreateProgram();
                glAttachShader(program, vertex);
                glAttachShader(program, fragment);
                const char *attributes[] = {"position", "color", "uv", "normal", "highlight"};
                for (unsigned i = 0; i < 5; ++i)
                    glBindAttribLocation(program, i, attributes[i]);
                glBindFragDataLocation(program, 0, "out_color");
                glBindFragDataLocation(program, 1, "out_id");
                glLinkProgram(program);
                glDeleteShader(vertex);
                glDeleteShader(fragment);
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
                    for (auto &entry : textures_)
                        glDeleteTextures(1, &entry.second.texture);
                    if (white_)
                        glDeleteTextures(1, &white_);
                    geometries_.clear();
                    textures_.clear();
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
                    if (!data->primitive_ids().empty()) {
                        glGenBuffers(1, &resource.primitive_buffer);
                        glBindBuffer(GL_TEXTURE_BUFFER, resource.primitive_buffer);
                        glBufferData(GL_TEXTURE_BUFFER, GLsizeiptr(data->primitive_ids().size() * sizeof(uint32_t)),
                                     data->primitive_ids().data(), GL_STATIC_DRAW);
                        glGenTextures(1, &resource.primitive_texture);
                        glBindTexture(GL_TEXTURE_BUFFER, resource.primitive_texture);
                        glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, resource.primitive_buffer);
                    }
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

                std::uint64_t uploads() const {
                    return uploads_;
                }

            private:
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
            unsigned errors = 0;
            GLuint program = 0;
            ResourceCache resources;
            FrameTarget target;

            ~Impl() {
                if (program)
                    glDeleteProgram(program);
            }

            void require_context() {
                const GLubyte *version = glGetString(GL_VERSION);
                if (!version)
                    throw std::runtime_error("OpenGL RenderDevice requires a current context");
            }

            [[noreturn]] void reject(const char *message) {
                ++errors;
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
                if (!finite(d.model_view_projection) || !finite(d.normal_matrix) || !finite(d.model) ||
                    !finite(d.eye) || !finite(d.light) || !finite(d.ambient) || !finite(d.specular) ||
                    !finite(d.back_color) || !finite(d.color) || !finite(d.shadow_matrix))
                    reject("draw command contains non-finite rendering state");
                if (d.color.w < 0 || d.color.w > 1)
                    reject("draw alpha must be in [0,1]");
                if (d.overlay_depth_test && !d.overlay)
                    reject("scene-depth annotation requires overlay=true");
                if (d.overlay_independent_depth) {
                    if (!d.overlay || d.overlay_depth_test)
                        reject("independent-depth HUD requires overlay=true and excludes scene-depth testing");
                    reject("independent-depth HUD is not supported by this OpenGL RenderDevice increment");
                }
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
                if (d.point_style != PointStyle::plain || d.line_style != LineStyle::plain)
                    reject("analytic point/line impostors are not supported by this OpenGL RenderDevice increment");
                if (!std::isfinite(d.point_size) || d.point_size <= 0)
                    reject("point size must be positive and finite");
                if (!std::isfinite(d.line_width) || d.line_width <= 0 ||
                    (d.line_width != 1 && d.geometry->topology() == Topology::lines))
                    reject("this OpenGL RenderDevice increment supports native one-pixel lines only");
                if (d.object_id == UINT32_MAX)
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

            std::vector<DrawCommand> expand_points(const DrawCommand &d, uint32_t width, uint32_t height) {
                resources.realize_points(d.geometry->point_data());
                std::vector<DrawCommand> result;
                const auto &source = d.geometry->vertices();
                const auto &indices = d.geometry->indices();
                const size_t count = indices.empty() ? source.size() : indices.size();
                std::vector<Vertex> vertices;
                std::vector<uint32_t> expanded_indices;
                std::vector<uint32_t> ids;
                vertices.reserve(count * 4);
                expanded_indices.reserve(count * 6);
                ids.reserve(count * 2);
                for (size_t i = 0; i < count; ++i) {
                    const Vertex &v = source[indices.empty() ? i : indices[i]];
                    const vec4 clip = d.model_view_projection * vec4(v.position, 1);
                    if (!(clip.w > 0) || !std::isfinite(clip.x / clip.w) || !std::isfinite(clip.y / clip.w))
                        continue;
                    const float x = float(d.point_size) / float(width);
                    const float y = float(d.point_size) / float(height);
                    const float cx = clip.x / clip.w, cy = clip.y / clip.w, cz = clip.z / clip.w;
                    const uint32_t base = uint32_t(vertices.size());
                    for (const vec2 &corner : {vec2(cx - x, cy - y), vec2(cx + x, cy - y),
                                               vec2(cx + x, cy + y), vec2(cx - x, cy + y)}) {
                        Vertex corner_vertex = v;
                        corner_vertex.position = vec3(corner.x, corner.y, cz);
                        vertices.push_back(corner_vertex);
                    }
                    for (uint32_t index : {0u, 1u, 2u, 0u, 2u, 3u})
                        expanded_indices.push_back(base + index);
                    const uint32_t id = d.geometry->primitive_ids().empty()
                                            ? uint32_t(i) : d.geometry->primitive_ids()[i];
                    ids.push_back(id);
                    ids.push_back(id);
                }
                DrawCommand expanded = d;
                expanded.geometry = std::make_shared<Geometry>(std::move(vertices), std::move(expanded_indices),
                                                              Topology::triangles, std::move(ids));
                expanded.model_view_projection = mat4::identity();
                expanded.point_style = PointStyle::plain;
                result.push_back(std::move(expanded));
                return result;
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
                    if (d.geometry->topology() == Topology::points) {
                        auto expanded = expand_points(d, width, height);
                        opaque.insert(opaque.end(), std::make_move_iterator(expanded.begin()),
                                      std::make_move_iterator(expanded.end()));
                    } else if (d.transparent) {
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

            void set_uniforms(const DrawCommand &d, bool ignore_highlight = false) {
                const GLuint p = program;
                auto location = [p](const char *name) { return glGetUniformLocation(p, name); };
                glUniformMatrix4fv(location("mvp"), 1, GL_FALSE, static_cast<const float *>(d.model_view_projection));
                glUniformMatrix4fv(location("model"), 1, GL_FALSE, static_cast<const float *>(d.model));
                glUniformMatrix3fv(location("normal_matrix"), 1, GL_FALSE, static_cast<const float *>(d.normal_matrix));
                glUniform4fv(location("color"), 1, d.color.data());
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

            void draw(const DrawCommand &d, uint32_t width, uint32_t height, bool color_pass, bool transparent) {
                const ivec4 viewport = viewport_of(d, width, height);
                const int x = viewport.x, y = int(height) - viewport.y - viewport.w;
                glViewport(x, y, viewport.z, viewport.w);
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
                set_uniforms(d);
                auto &resource = resources.geometry(d.geometry);
                glBindVertexArray(resource.vao);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, resources.texture(d.texture));
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_BUFFER, resource.primitive_texture);
                glActiveTexture(GL_TEXTURE0);
                const GLenum mode = topology(d.geometry->topology());
                if (resource.indices)
                    glDrawElements(mode, GLsizei(d.geometry->indices().size()), GL_UNSIGNED_INT, nullptr);
                else
                    glDrawArrays(mode, 0, GLsizei(d.geometry->vertices().size()));
            }

            void draw_scene(const std::vector<DrawCommand> &commands, uint32_t width, uint32_t height,
                            bool include_color, bool include_ids) {
                glUseProgram(program);
                glDisable(GL_CULL_FACE);
                glEnable(GL_DEPTH_TEST);
                if (include_color) {
                    target.color().bind();
                    for (const auto &d : commands)
                        draw(d, width, height, true, d.transparent);
                }
                if (include_ids) {
                    target.scene().bind();
                    target.scene().activate_draw_buffers(0, 1);
                    for (const auto &d : commands)
                        draw(d, width, height, false, false);
                }
                glDisable(GL_SCISSOR_TEST);
            }

            void draw_overlays(const std::vector<DrawCommand> &commands, uint32_t width, uint32_t height) {
                std::vector<const DrawCommand *> flat, depth_tested;
                for (const auto &d : commands) {
                    if (!d.overlay)
                        continue;
                    (d.overlay_depth_test ? depth_tested : flat).push_back(&d);
                }
                auto &output = target.output();
                output.bind();
                output.activate_draw_buffer(0);
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glDisable(GL_SCISSOR_TEST);
                glUseProgram(program);
                for (int phase = 0; phase < 2; ++phase) {
                    glDepthMask(GL_FALSE);
                    if (phase == 0) {
                        glEnable(GL_DEPTH_TEST);
                        glDepthFunc(GL_LEQUAL);
                    } else {
                        glDisable(GL_DEPTH_TEST);
                    }
                    glEnable(GL_BLEND);
                    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                    for (const DrawCommand *d : phase == 0 ? depth_tested : flat) {
                        const ivec4 viewport = viewport_of(*d, width, height);
                        const int x = viewport.x, y = int(height) - viewport.y - viewport.w;
                        glViewport(x, y, viewport.z, viewport.w);
                        glEnable(GL_SCISSOR_TEST);
                        glScissor(x, y, viewport.z, viewport.w);
                        set_uniforms(*d, true);
                        auto &resource = resources.geometry(d->geometry);
                        glBindVertexArray(resource.vao);
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, resources.texture(d->texture));
                        if (resource.indices)
                            glDrawElements(GL_TRIANGLES, GLsizei(d->geometry->indices().size()), GL_UNSIGNED_INT, nullptr);
                        else
                            glDrawArrays(GL_TRIANGLES, 0, GLsizei(d->geometry->vertices().size()));
                    }
                }
                glDisable(GL_SCISSOR_TEST);
                glDepthMask(GL_TRUE);
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
                    program = create_program();
                target.ensure(width, height, samples);
                clear_scene(background);
                const auto scene = prepare(commands, width, height);
                draw_scene(scene, width, height, include_color, true);
                if (include_color) {
                    auto &color = target.color();
                    auto &output = target.output();
                    color.bind(GL_READ_FRAMEBUFFER);
                    output.bind(GL_DRAW_FRAMEBUFFER);
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, color.handle());
                    glReadBuffer(GL_COLOR_ATTACHMENT0);
                    glBlitFramebuffer(0, 0, int(width), int(height), 0, 0, int(width), int(height),
                                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
                    if (overlays)
                        draw_overlays(commands, width, height);
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
                ++impl_->errors;
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

        bool OpenGLDevice::present(uint32_t, uint32_t, const std::vector<DrawCommand> &, const vec4 &, unsigned,
                                   TransparencyMethod, unsigned) {
            throw std::logic_error("OpenGL RenderDevice created by create_opengl_device() is headless");
        }

        unsigned OpenGLDevice::validation_errors() const {
            return impl_->errors;
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

        std::unique_ptr<RenderDevice> create_opengl_device() {
            return std::unique_ptr<RenderDevice>(new OpenGLDevice);
        }

    } // namespace graphics
} // namespace easy3d
