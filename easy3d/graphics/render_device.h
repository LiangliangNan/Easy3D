// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Liangliang Nan
#ifndef EASY3D_GRAPHICS_RENDER_DEVICE_H
#define EASY3D_GRAPHICS_RENDER_DEVICE_H

#include <easy3d/core/types.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace easy3d { namespace graphics {

// These descriptions deliberately contain no OpenGL or Vulkan handles.
enum class Topology { triangles, lines, points };

// Immutable positions in point draw/instance order, with original source slots.
// Devices realize this once as a shared vertex/storage buffer. Color, normal,
// texture, highlighting and transform changes can retain the same snapshot.
class PointData {
public:
    explicit PointData(std::vector<vec3> positions, std::vector<std::uint32_t> source_ids={},
                       std::uint32_t source_count=0);
    PointData& operator=(const PointData&)=delete;
    const std::vector<vec3>& positions() const { return positions_; }
    const std::vector<std::uint32_t>& source_ids() const { return source_ids_; }
    std::uint32_t source_count() const { return source_count_; }
private:
    std::vector<vec3> positions_;
    std::vector<std::uint32_t> source_ids_;
    std::uint32_t source_count_;
};

struct Vertex {
    vec3 position;
    vec3 normal = vec3(0, 0, 1);
    vec4 color = vec4(1, 1, 1, 1);
    vec2 texcoord = vec2(0, 0);
    // RGB selection tint and mix strength [0,1], applied after texture/lighting.
    // Set the same value on all corners of a primitive. Overlays ignore this.
    vec4 highlight = vec4(0);
};

// Immutable snapshots. Input is copied, so temporaries and later model edits are safe.
// Updates create a new snapshot; devices retire old GPU realizations after submission.
class Geometry {
public:
    Geometry(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices = {},
             Topology topology = Topology::triangles,
             std::vector<std::uint32_t> primitive_ids = {},
             std::shared_ptr<const PointData> point_data = {});
    const std::vector<Vertex>& vertices() const { return vertices_; }
    const std::vector<std::uint32_t>& indices() const { return indices_; }
    const std::vector<std::uint32_t>& primitive_ids() const { return primitive_ids_; }
    Topology topology() const { return topology_; }
    const std::shared_ptr<const PointData>& point_data() const { return point_data_; }
private:
    const std::vector<Vertex> vertices_;
    const std::vector<std::uint32_t> indices_;
    const Topology topology_;
    const std::vector<std::uint32_t> primitive_ids_;
    std::shared_ptr<const PointData> point_data_;
};

enum class TextureWrap { clamp_to_edge, repeat };
enum class TextureFilter { nearest, linear };
struct Sampler {
    TextureWrap wrap = TextureWrap::clamp_to_edge;
    TextureFilter filter = TextureFilter::nearest;
};

class Image {
public:
    // RGBA8, row zero at the top. No implicit global image-loader flip state.
    Image(std::uint32_t width, std::uint32_t height, std::vector<unsigned char> rgba, Sampler sampler = {});
    std::uint32_t width() const { return width_; }
    std::uint32_t height() const { return height_; }
    const std::vector<unsigned char>& rgba() const { return rgba_; }
    const Sampler& sampler() const { return sampler_; }
private:
    const std::uint32_t width_, height_;
    const std::vector<unsigned char> rgba_;
    const Sampler sampler_;
};

enum class PointStyle { plain, sphere, surfel };
enum class LineStyle { plain, cylinder, cone };

struct DrawCommand {
    std::shared_ptr<const Geometry> geometry;
    std::shared_ptr<const Image> texture; // null uses a valid white texture
    // Existing Easy3D/OpenGL camera convention: Y up, clip depth [-w, w].
    // Backend conversion to Vulkan is applied exactly once in the vertex shader.
    mat4 model_view_projection = mat4::identity();
    mat3 normal_matrix = mat3::identity(); // inverse-transpose of the model transform
    vec4 color = vec4(1, 1, 1, 1);
    std::uint32_t object_id = 0;
    bool lighting = false; // fixed world-space directional diffuse lighting
    // Straight-alpha triangle annotations composite after the 3D scene.
    // They never write scene depth or picking IDs; depth testing is opt-in below.
    // Flat/scene-depth annotations are unlit; independent 3D HUDs support lighting.
    bool overlay = false;
    // Depth-tested alpha annotations render after the opaque scene and before
    // screen overlays. They test scene depth (<=), but never write depth or IDs.
    // Supply these in back-to-front order when they overlap; triangles only.
    bool overlay_depth_test = false;
    // 3D HUD triangles share a separately cleared depth attachment, testing and
    // writing their own depth without touching scene depth or IDs. They render
    // after scene annotations and before flat HUD overlays. Mutually exclusive
    // with overlay_depth_test; requires overlay=true. Lighting is supported.
    bool overlay_independent_depth = false;
    // Flat triangle HUD samples the current frame's center-sample scene depth
    // as grayscale through its UVs. Requires overlay=true, no texture and no
    // overlay depth flags. Sampling never changes scene depth or picking IDs.
    bool scene_depth_texture = false;
    // Up to two view-space halfspaces: dot(plane, vec4(view_position,1)) >= 0.
    // When clipping is active, model_view must map the geometry into view space
    // and be affine for every topology, consistently with model_view_projection.
    // Triangles/line centerlines clip geometrically; plain points test centers;
    // analytic impostors clip their actual front surface. Overlays ignore planes.
    std::vector<vec4> clipping_planes;
    bool discard_clipped_primitives = false; // reject if any control vertex is outside
    // Plain points use framebuffer-pixel diameters. Spheres and surfels use view-space radii,
    // after transforming their centers; model scaling does not scale the radius.
    PointStyle point_style = PointStyle::plain;
    float point_size = 1;
    float sphere_radius = 1; // shared radius for sphere/surfel impostors
    vec3 sphere_light = vec3(0.3f, 0.5f, 1); // view-space directional diffuse light for all impostors
    // Surfels orient their disc by the inverse-transpose of this affine matrix's
    // linear part, using Vertex::normal (nonzero). Radius stays independent of scale.
    mat4 model_view = mat4::identity();
    mat4 projection = mat4::identity(); // OpenGL convention; all impostors
    // Line impostors transform both endpoints with model_view. Their radius is
    // in view-space units, independent of model scale. Cones taper from the
    // first endpoint (base) to the second (tip); cylinders have two flat caps.
    LineStyle line_style = LineStyle::plain;
    float line_radius = 1;
    // Plain lines use framebuffer-pixel widths. Width one retains native line
    // rasterization; other widths use butt-capped triangle ribbons, without joins.
    float line_width = 1;
};

// Validates and normalizes finite plane equations with nonzero normals.
std::vector<vec4> normalize_clipping_planes(const std::vector<vec4>& planes);

// Top-left framebuffer-pixel coordinates to the shared OpenGL clip convention.
mat4 screen_space_projection(std::uint32_t width, std::uint32_t height);

struct FrameImage {
    std::uint32_t width = 0, height = 0;
    std::vector<unsigned char> rgba;  // tightly packed RGBA8, top row first
    std::vector<float> depth;        // [0, 1], clear depth is 1
    struct Id { std::uint32_t object, primitive; };
    // UINT32_MAX in both fields denotes background. No normalized-color encoding.
    std::vector<Id> ids;
};

struct DeviceOptions {
    // Explicitly requested validation must be available, otherwise creation fails.
    bool validation = false;
    bool synchronization_validation = false;
    // Optional physical-device name substring; useful for reproducible CI runs.
    std::string device_name;
};

// GPU through-selection of projected point centers in a top-left framebuffer
// polygon. Polygon crossings use the even-odd rule (boundary included). It is
// independent of rasterization/occlusion, point style, MSAA and scene readback.
// PointData source IDs address bits; empty words start from zero, otherwise supply
// ceil(source_count/32) words. Unmatched/padding bits are preserved. Nonfinite
// points are skipped (usable for deleted vertex slots). Camera MVP uses [-w,w]
// depth; centers outside its frustum or local halfspaces are skipped.
struct PointSelectionRequest {
    std::shared_ptr<const PointData> points;
    std::vector<vec2> polygon;
    std::vector<std::uint32_t> words;
    mat4 model_view_projection = mat4::identity();
    std::uint32_t width=0, height=0;
    bool deselect=false;
    std::vector<vec4> local_clipping_planes; // up to two, keep dot(plane,[point,1])>=0
};

class RenderDevice {
public:
    virtual ~RenderDevice() = default;
    virtual const std::string& name() const = 0;
    // Bit mask using the numeric sample counts (1, 2, 4, ...). Color MSAA uses
    // supported RGBA8/D32 counts; integer IDs/depth picking stay single-sampled.
    // Sphere/surfel MSAA additionally requires sampleRateShading. Query by style before
    // offering multisampled impostors on devices without that optional feature.
    virtual std::uint32_t supported_sample_counts(PointStyle style = PointStyle::plain) const = 0;
    // Cylinder/cone MSAA has the same sampleRateShading requirement.
    virtual std::uint32_t supported_sample_counts(LineStyle style) const = 0;
    // Synchronous offscreen capture. No window, GL context, surface or swapchain.
    // Instances are externally synchronized; separate instances are independent.
    virtual FrameImage render(std::uint32_t width, std::uint32_t height,
                              const std::vector<DrawCommand>& commands,
                              const vec4& background = vec4(1, 1, 1, 1), unsigned samples = 1) = 0;
    // Queue a frame for a configured presentation surface, without CPU readback.
    // False means zero extent/out-of-date surface; the host should retry after events.
    // Headless devices report an unsupported operation explicitly.
    virtual bool present(std::uint32_t width, std::uint32_t height,
                         const std::vector<DrawCommand>& commands,
                         const vec4& background = vec4(1, 1, 1, 1), unsigned samples = 1) = 0;
    virtual unsigned validation_errors() const = 0;
    // Synchronous GPU polygon upload, compute dispatch and selection readback.
    // Externally synchronized like render(); no render targets are modified.
    virtual std::vector<std::uint32_t> select_points(const PointSelectionRequest& request) = 0;
    // Cumulative position-resource realizations, useful for transfer diagnostics.
    // Rendering and selection share these uploads; attribute uploads are separate.
    virtual std::uint64_t point_upload_count() const = 0;
};

// Implemented by easy3d::vulkan. Throws std::runtime_error on unavailable devices.
std::unique_ptr<RenderDevice> create_vulkan_device(const DeviceOptions& options = {});

} } // namespace easy3d::graphics
#endif
