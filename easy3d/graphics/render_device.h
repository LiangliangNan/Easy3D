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

struct Vertex {
    vec3 position;
    vec3 normal = vec3(0, 0, 1);
    vec4 color = vec4(1, 1, 1, 1);
    vec2 texcoord = vec2(0, 0);
};

// Immutable snapshots. Input is copied, so temporaries and later model edits are safe.
// Updates create a new snapshot; devices retire old GPU realizations after submission.
class Geometry {
public:
    Geometry(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices = {},
             Topology topology = Topology::triangles,
             std::vector<std::uint32_t> primitive_ids = {});
    const std::vector<Vertex>& vertices() const { return vertices_; }
    const std::vector<std::uint32_t>& indices() const { return indices_; }
    const std::vector<std::uint32_t>& primitive_ids() const { return primitive_ids_; }
    Topology topology() const { return topology_; }
private:
    const std::vector<Vertex> vertices_;
    const std::vector<std::uint32_t> indices_;
    const Topology topology_;
    const std::vector<std::uint32_t> primitive_ids_;
};

class Image {
public:
    // RGBA8, row zero at the top. No implicit global image-loader flip state.
    Image(std::uint32_t width, std::uint32_t height, std::vector<unsigned char> rgba);
    std::uint32_t width() const { return width_; }
    std::uint32_t height() const { return height_; }
    const std::vector<unsigned char>& rgba() const { return rgba_; }
private:
    const std::uint32_t width_, height_;
    const std::vector<unsigned char> rgba_;
};

enum class PointStyle { plain, sphere, surfel };

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
    // Plain points use framebuffer-pixel diameters. Spheres and surfels use view-space radii,
    // after transforming their centers; model scaling does not scale the radius.
    PointStyle point_style = PointStyle::plain;
    float point_size = 1;
    float sphere_radius = 1; // shared radius for sphere/surfel impostors
    vec3 sphere_light = vec3(0.3f, 0.5f, 1); // view-space directional diffuse light
    // Surfels orient their disc by the inverse-transpose of this affine matrix's
    // linear part, using Vertex::normal (nonzero). Radius stays independent of scale.
    mat4 model_view = mat4::identity();
    mat4 projection = mat4::identity(); // OpenGL convention; sphere/surfel impostors
};

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

class RenderDevice {
public:
    virtual ~RenderDevice() = default;
    virtual const std::string& name() const = 0;
    // Bit mask using the numeric sample counts (1, 2, 4, ...). Color MSAA uses
    // supported RGBA8/D32 counts; integer IDs/depth picking stay single-sampled.
    // Sphere/surfel MSAA additionally requires sampleRateShading. Query by style before
    // offering multisampled impostors on devices without that optional feature.
    virtual std::uint32_t supported_sample_counts(PointStyle style = PointStyle::plain) const = 0;
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
};

// Implemented by easy3d::vulkan. Throws std::runtime_error on unavailable devices.
std::unique_ptr<RenderDevice> create_vulkan_device(const DeviceOptions& options = {});

} } // namespace easy3d::graphics
#endif
