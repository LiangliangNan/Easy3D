// SPDX-License-Identifier: GPL-3.0-or-later
/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/
// Copyright (C) 2026 Liangliang Nan
#ifndef EASY3D_RENDERER_RENDER_DEVICE_H
#define EASY3D_RENDERER_RENDER_DEVICE_H

#include <easy3d/core/types.h>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace easy3d {
    namespace graphics {

        /// Primitive descriptions deliberately contain no OpenGL or Vulkan handles.
        enum class Topology { triangles, lines, points };

        /**
 * \brief Immutable point positions in draw order, mapped to original source slots.
 * \details Native OpenGL snapshots may contain GPU-only storage (empty CPU
 * vectors) and are valid only for selection on their owning viewer. The source
 * count remains available. Devices realize CPU snapshots once as a shared
 * vertex/storage buffer, reusable when attributes or transforms change.
 */
        class PointData {
        public:
            /**
     * \brief Create a point-data snapshot.
     * \param positions Point centers in draw/instance order.
     * \param source_ids Original source slots corresponding to each point.
     *        An empty vector indicates identity ordering.
     * \param source_count Number of original source slots, including absent slots.
     */
            explicit PointData(std::vector<vec3> positions,
                               std::vector<std::uint32_t> source_ids = {},
                               std::uint32_t source_count = 0);
            /// Destroy the point-data snapshot.
            virtual ~PointData() = default;
            /// Copy assignment is disabled; snapshots are shared by const ownership.
            PointData &operator=(const PointData &) = delete;
            /// Return positions in draw/instance order.
            /// \return Point centers in draw/instance order.
            const std::vector<vec3> &positions() const {
                return positions_;
            }
            /// Return the original source slot for each position, or an empty vector for identity order.
            /// \return Original source slots, or an empty vector for identity ordering.
            const std::vector<std::uint32_t> &source_ids() const {
                return source_ids_;
            }
            /// Return the number of source slots represented by this snapshot.
            /// \return Number of original source slots.
            std::uint32_t source_count() const {
                return source_count_;
            }

        private:
            std::vector<vec3> positions_;
            std::vector<std::uint32_t> source_ids_;
            std::uint32_t source_count_;
        };

        /// Per-vertex data consumed by a draw command.
        struct Vertex {
            /// Position in geometry-local coordinates.
            vec3 position;
            /// Surface normal.
            vec3 normal = vec3(0, 0, 1);
            /// RGBA vertex color.
            vec4 color = vec4(1, 1, 1, 1);
            /// Texture coordinate.
            vec2 texcoord = vec2(0, 0);
            /// RGB selection tint and mix strength in [0,1], applied after texture and
            /// lighting. Set consistently on a primitive's corners; overlays ignore it.
            vec4 highlight = vec4(0);
        };

        /**
 * \brief Immutable geometry snapshot submitted to a RenderDevice.
 * \details Constructor inputs are copied, so temporaries and subsequent model
 * edits are safe. Updating geometry creates a new snapshot.
 */
        class Geometry {
        public:
            /**
     * \brief Create a geometry snapshot.
     * \param vertices Per-vertex positions and attributes.
     * \param indices Optional indices into vertices.
     * \param topology Primitive topology.
     * \param primitive_ids Optional per-primitive picking IDs.
     * \param point_data Optional point positions and source-slot mapping.
     */
            Geometry(std::vector<Vertex> vertices,
                     std::vector<std::uint32_t> indices = {},
                     Topology topology = Topology::triangles,
                     std::vector<std::uint32_t> primitive_ids = {},
                     std::shared_ptr<const PointData> point_data = {});
            /// Return the vertex data.
            /// \return Vertex positions and attributes.
            const std::vector<Vertex> &vertices() const {
                return vertices_;
            }
            /// Return indices, or an empty vector for non-indexed geometry.
            /// \return Indices, or an empty vector for non-indexed geometry.
            const std::vector<std::uint32_t> &indices() const {
                return indices_;
            }
            /// Return per-primitive IDs, if supplied.
            /// \return Per-primitive picking IDs, if supplied.
            const std::vector<std::uint32_t> &primitive_ids() const {
                return primitive_ids_;
            }
            /// Return the geometry topology.
            /// \return Primitive topology.
            Topology topology() const {
                return topology_;
            }
            /// Return associated point data, if present.
            /// \return Associated point snapshot, or an empty pointer.
            const std::shared_ptr<const PointData> &point_data() const {
                return point_data_;
            }

        private:
            const std::vector<Vertex> vertices_;
            const std::vector<std::uint32_t> indices_;
            const Topology topology_;
            const std::vector<std::uint32_t> primitive_ids_;
            std::shared_ptr<const PointData> point_data_;
        };

        enum class TextureWrap { clamp_to_edge, repeat };
        enum class TextureFilter { nearest, linear };

        /// Texture addressing and filtering configuration.
        struct Sampler {
            /// Addressing mode for texture coordinates outside the image.
            TextureWrap wrap = TextureWrap::clamp_to_edge;
            /// Interpolation used when sampling between texels.
            TextureFilter filter = TextureFilter::nearest;
        };

        /// Immutable RGBA8 image with explicit sampler state.
        class Image {
        public:
            /**
     * \brief Create an RGBA8 image snapshot.
     * \param width Image width in pixels.
     * \param height Image height in pixels.
     * \param rgba Tightly packed pixels, with the top row first.
     * \param sampler Texture addressing and filtering options.
     * \details No implicit global image-loader flip state is used.
     */
            Image(std::uint32_t width, std::uint32_t height, std::vector<unsigned char> rgba, Sampler sampler = {});
            /// Return image width in pixels.
            /// \return Image width in pixels.
            std::uint32_t width() const {
                return width_;
            }
            /// Return image height in pixels.
            /// \return Image height in pixels.
            std::uint32_t height() const {
                return height_;
            }
            /// Return tightly packed RGBA8 data, with the top row first.
            /// \return Tightly packed RGBA8 pixels, top row first.
            const std::vector<unsigned char> &rgba() const {
                return rgba_;
            }
            /// Return the image sampler settings.
            /// \return Texture addressing and filtering options.
            const Sampler &sampler() const {
                return sampler_;
            }

        private:
            const std::uint32_t width_, height_;
            const std::vector<unsigned char> rgba_;
            const Sampler sampler_;
        };

        enum class PointStyle { plain, sphere, surfel };
        enum class LineStyle { plain, cylinder, cone };

        /**
 * \brief Complete backend-independent description of a draw operation.
 * \details Draw commands carry immutable geometry and explicit rendering state.
 * Backends interpret matrices using the documented Easy3D/OpenGL convention.
 */
        struct DrawCommand {
            std::shared_ptr<const Geometry> geometry; ///< Geometry snapshot to draw.
            std::shared_ptr<const Image> texture;     ///< Optional texture; null selects white.
            // Existing Easy3D/OpenGL camera convention: Y up, clip depth [-w, w].
            // Backend conversion to Vulkan is applied exactly once in the vertex shader.
            mat4 model_view_projection = mat4::identity(); ///< Model-to-clip transform in Easy3D/OpenGL convention.
            mat3 normal_matrix = mat3::identity();         ///< Inverse-transpose normal transform.
            vec4 color = vec4(1, 1, 1, 1);                 ///< Uniform RGBA multiplier.
            std::uint32_t object_id = 0;                   ///< Object ID written for picking.
            // Top-left framebuffer pixels. All zeros select the complete render target.
            // The backend uses this same rectangle for rasterization and clipping.
            ivec4 viewport = ivec4(0); ///< Top-left framebuffer viewport; zero selects the full target.
            bool lighting = false;     ///< Enable fixed directional diffuse lighting.
            // Renderer material state. Low-level commands retain basic diffuse lighting
            // unless this is enabled; drawable adapters forward the shared State here.
            bool material_lighting = false;           ///< Enable Renderer material lighting.
            bool lighting_two_sides = false;          ///< Apply material lighting to both triangle sides.
            bool distinct_back_color = false;         ///< Use back_color for back-facing triangles.
            mat4 model = mat4::identity();            ///< Model-to-world transform.
            vec3 eye = vec3(0);                       ///< Eye position for material lighting.
            vec3 light = vec3(0, 0, 1);               ///< Directional light for material lighting.
            vec4 ambient = vec4(.05f);                ///< Ambient material color.
            vec4 specular = vec4(.4f);                ///< Specular material color.
            vec4 back_color = vec4(.8f, .4f, .4f, 1); ///< Back-face color when enabled.
            float shininess = 64;                     ///< Specular exponent.
            // Straight-alpha triangle annotations composite after the 3D scene.
            // They never write scene depth or picking IDs; depth testing is opt-in below.
            // Flat/scene-depth annotations are unlit; independent 3D HUDs support lighting.
            bool overlay =
                false; ///< Composite triangle annotations after the scene; overlays do not write scene depth or IDs.
            // Depth-tested alpha annotations render after the opaque scene and before
            // screen overlays. They test scene depth (<=), but never write depth or IDs.
            // Supply these in back-to-front order when they overlap; triangles only.
            bool overlay_depth_test =
                false; ///< Test triangle overlays against scene depth without writing depth or IDs; submit overlapping annotations back-to-front.
            // 3D HUD triangles share a separately cleared depth attachment, testing and
            // writing their own depth without touching scene depth or IDs. They render
            // after scene annotations and before flat HUD overlays. Mutually exclusive
            // with overlay_depth_test; requires overlay=true. Lighting is supported.
            bool overlay_independent_depth =
                false; ///< Use an independent depth buffer for 3D HUD triangles; requires overlay and excludes overlay_depth_test.
            // Flat triangle HUD samples the current frame's center-sample scene depth
            // as grayscale through its UVs. Requires overlay=true, no texture and no
            // overlay depth flags. Sampling never changes scene depth or picking IDs.
            bool scene_depth_texture =
                false; ///< Sample center-sample scene depth as grayscale through UVs; requires an untextured overlay without overlay depth flags.
            // Straight-alpha scene triangles composite over the opaque scene, tested and
            // written against the shared scene depth and IDs. They draw after opaque
            // geometry in this vector, ordered far to near by the backend. color.w is
            // the opacity. Triangle geometry only; overlays have their own blending.
            bool transparent =
                false; ///< Blend scene triangles using color.w opacity; writes scene depth and picking IDs.
            // Full-screen multiplicative shading from this frame's scene depth. Requires
            // overlay=true, no texture, no overlay depth flags. Geometry must be a quad
            // covering the viewport with the depth-texture UV convention; color carries
            // (near plane, far plane, scene size, unused). Background pixels keep their
            // color because their neighbor depth difference is zero.
            bool eye_dome_lighting =
                false; ///< Apply eye-dome lighting from scene depth; requires a full-viewport untextured overlay quad.
            // Full-screen multiplicative ambient occlusion from this frame's scene depth.
            // Requires overlay=true, no texture, no overlay depth flags. Geometry must be
            // a quad covering the viewport with the depth-texture UV convention; color
            // carries (view-space radius, bias, unused, unused) and projection must be the
            // scene's OpenGL-convention projection matrix. Background pixels keep their
            // color, and the pass never changes scene depth or picking IDs.
            bool ambient_occlusion =
                false; ///< Apply screen-space ambient occlusion; color carries view-space radius and bias.
            // Scene triangles that are additionally rasterized into this frame's shadow
            // map before the scene. Requires triangles, a finite shadow_matrix and a
            // positive, power-of-two shadow_map_size. They render normally as well.
            bool shadow_caster = false; ///< Rasterize scene triangles into the shadow map as well as the scene.
            // Scene triangles that only receive this frame's shadow: they keep the
            // background ID (object_id must be UINT32_MAX) and are lit normally, so the
            // virtual background plane can occlude without appearing in picking.
            bool shadow_background = false; ///< Receive shadows while retaining the background picking ID.
            // Full-screen multiplicative shadow overlay. Requires overlay=true, no
            // texture, no overlay depth flags. Geometry must cover the viewport with the
            // depth-texture UV convention. Samples the frame's shadow map through the
            // shadow matrix; color carries (darkness, softness, light near, light far).
            // Softness zero compares depths directly; positive values filter blockers.
            // Never changes scene depth or picking IDs.
            bool shadow =
                false; ///< Apply a full-viewport multiplicative shadow overlay; color carries darkness, softness, light near, and light far.
            // Camera clip space -> shadow texture coordinates. Scene casters also use
            // it to map their vertices into the light view.
            mat4 shadow_matrix = mat4::identity(); ///< Camera-clip to shadow-texture transform.
            unsigned shadow_map_size = 1024;       ///< Shadow-map side length in pixels.
            // Up to two view-space halfspaces: dot(plane, vec4(view_position,1)) >= 0.
            // When clipping is active, model_view must map the geometry into view space
            // and be affine for every topology, consistently with model_view_projection.
            // Triangles/line centerlines clip geometrically; plain points test centers;
            // analytic impostors clip their actual front surface. Overlays ignore planes.
            std::vector<vec4>
                clipping_planes; ///< Up to two view-space halfspaces retaining dot(plane, [position,1]) >= 0; overlays ignore them.
            bool discard_clipped_primitives = false; ///< Reject a primitive if any control vertex is outside.
            // Plain points use framebuffer-pixel diameters. Spheres and surfels use view-space radii,
            // after transforming their centers; model scaling does not scale the radius.
            PointStyle point_style = PointStyle::plain; ///< Point rendering style.
            float point_size = 1;                       ///< Plain-point diameter in framebuffer pixels.
            float sphere_radius = 1; ///< View-space radius for sphere/surfel impostors, independent of model scale.
            vec3 sphere_light = vec3(0.3f, 0.5f, 1); ///< View-space directional light for point impostors.
            // Surfels orient their disc by the inverse-transpose of this affine matrix's
            // linear part, using Vertex::normal (nonzero). Radius stays independent of scale.
            mat4 model_view = mat4::identity(); ///< Model-to-view transform for impostor geometry.
            mat4 projection = mat4::identity(); ///< OpenGL-convention projection for impostor geometry.
            // Line impostors transform both endpoints with model_view. Their radius is
            // in view-space units, independent of model scale. Cones taper from the
            // first endpoint (base) to the second (tip); cylinders have two flat caps.
            LineStyle line_style = LineStyle::plain; ///< Line rendering style.
            float line_radius = 1; ///< View-space radius for cylinder/cone impostors, independent of model scale.
            // Plain lines use framebuffer-pixel widths. Width one retains native line
            // rasterization; other widths use butt-capped triangle ribbons, without joins.
            float line_width = 1; ///< Plain-line width in framebuffer pixels.
            // Filled triangles are pushed away from the camera, as the OpenGL viewer
            // does with glPolygonOffset, so coplanar line drawables stay visible.
            // Ignored for other topologies and overlays.
            bool depth_bias = false; ///< Offset filled triangles to expose coplanar lines.
        };

        // Validates and normalizes finite plane equations with nonzero normals.
        std::vector<vec4> normalize_clipping_planes(const std::vector<vec4> &planes);

        // Top-left framebuffer-pixel coordinates to the shared OpenGL clip convention.
        mat4 screen_space_projection(std::uint32_t width, std::uint32_t height);

        /**
 * \brief CPU-readable color, depth, and picking data captured from a frame.
 */
        struct FrameImage {
            /// Captured framebuffer dimensions in pixels.
            std::uint32_t width = 0;
            /// Captured framebuffer height in pixels.
            std::uint32_t height = 0;
            /// Tightly packed RGBA8 color pixels, top row first.
            std::vector<unsigned char> rgba;
            /// Per-pixel depth in [0,1]; clear depth is 1.
            std::vector<float> depth;
            /// Object and primitive IDs associated with one pixel.
            struct Id {
                /// Object ID, or UINT32_MAX for the background.
                std::uint32_t object;
                /// Primitive ID, or UINT32_MAX when provenance is unavailable.
                std::uint32_t primitive;
            };
            // UINT32_MAX object denotes background. A UINT32_MAX primitive on a foreground
            // object denotes unavailable primitive provenance (e.g. OpenGL Viewer capture).
            /// Per-pixel picking IDs; background objects use UINT32_MAX.
            std::vector<Id> ids;
        };

        /// Color/depth/ID result for one scene pixel.
        struct ScenePixel {
            /// Object and primitive IDs; background uses UINT32_MAX.
            FrameImage::Id id{UINT32_MAX, UINT32_MAX};
            /// Scene depth in [0,1], defaulting to the clear value.
            float depth = 1.0f;
        };

        // How the frame composites commands marked DrawCommand::transparent over the
        // opaque scene. All methods leave screen overlays and picking IDs unchanged.
        enum class TransparencyMethod {
            // Straight alpha, ordered far to near per drawable and per triangle. Writes
            // scene depth and IDs, so the nearest transparent surface is pickable.
            sorted,
            // Order-independent average color accumulation, matching the depth-free
            // legacy AverageColorBlending composite. Ignores opaque scene depth.
            average,
            // Order-independent front-to-back depth peeling, the depth-free legacy
            // DualDepthPeeling effect without its min-max (front and back) pairing.
            peeling,
            // Order-independent per-pixel linked lists (Yang et al., NVIDIA 2010).
            // Stores all fragments per pixel in GPU buffers, sorts by depth, and blends.
            // Provides perfect transparency quality regardless of surface intersections.
            // Requires more memory and is slower than peeling, but produces correct results.
            linked_lists
        };

        // GPU through-selection of projected point centers in a top-left framebuffer
        // polygon. Polygon crossings use the even-odd rule (boundary included). It is
        // independent of rasterization/occlusion, point style, MSAA and scene readback.
        // PointData source IDs address bits; empty words start from zero, otherwise supply
        // ceil(source_count/32) words. Unmatched/padding bits are preserved. Nonfinite
        // points are skipped (usable for deleted vertex slots). Camera MVP uses [-w,w]
        // depth; centers outside its frustum or local halfspaces are skipped.
        /**
 * \brief Inputs for GPU selection of point centers inside a screen polygon.
 * \details The polygon uses the even-odd crossing rule with its boundary
 * included. Selection is independent of rasterization, occlusion, point style,
 * MSAA, and scene readback. Nonfinite point positions are skipped.
 */
        struct PointSelectionRequest {
            /// Immutable point positions and source-slot mapping.
            std::shared_ptr<const PointData> points;
            /// Polygon in top-left framebuffer pixel coordinates.
            std::vector<vec2> polygon;
            /// Selection bitset, one bit per source slot.
            std::vector<std::uint32_t> words;
            /// Transform from point-local coordinates to OpenGL clip coordinates.
            mat4 model_view_projection = mat4::identity();
            /// Framebuffer width in pixels.
            std::uint32_t width = 0;
            /// Framebuffer height in pixels.
            std::uint32_t height = 0;
            /// Clear bits inside the polygon instead of setting them.
            bool deselect = false;
            /// Up to two local-space halfspaces that points must satisfy.
            std::vector<vec4> local_clipping_planes; // up to two, keep dot(plane,[point,1])>=0
        };

        /**
 * \brief Backend-independent rendering, capture, and selection interface.
 * \details Operations that synchronously access a device require external
 * synchronization per instance. Separate instances are independent.
 */
        class RenderDevice {
        public:
            /// Destroy the backend device.
            virtual ~RenderDevice() = default;
            /// Return a human-readable backend/device name.
            /// \return Backend/device name.
            virtual const std::string &name() const = 0;
            // Bit mask using the numeric sample counts (1, 2, 4, ...). Color MSAA uses
            // supported RGBA8/D32 counts; integer IDs/depth picking stay single-sampled.
            // Sphere/surfel MSAA additionally requires sampleRateShading. Query by style before
            // offering multisampled impostors on devices without that optional feature.
            /**
     * \brief Query supported MSAA sample counts for a point style.
     * \param style Point style whose shader requirements are checked.
     * \return Bit mask containing supported numeric sample counts.
     */
            virtual std::uint32_t supported_sample_counts(PointStyle style = PointStyle::plain) const = 0;
            // Cylinder/cone MSAA has the same sampleRateShading requirement.
            /**
     * \brief Query supported MSAA sample counts for a line style.
     * \param style Line style whose shader requirements are checked.
     * \return Bit mask containing supported numeric sample counts.
     */
            virtual std::uint32_t supported_sample_counts(LineStyle style) const = 0;
            // Synchronous offscreen capture. No window, GL context, surface or swapchain.
            // Instances are externally synchronized; separate instances are independent.
            // peel_layers bounds the front-to-back layers of TransparencyMethod::peeling;
            // zero asks for all layers, capped by the backend.
            /**
     * \brief Render commands into an offscreen framebuffer and read back results.
     * \param width Target width in pixels.
     * \param height Target height in pixels.
     * \param commands Draw commands for this frame.
     * \param background RGBA clear color.
     * \param samples Requested MSAA sample count.
     * \param transparency Method for transparent commands.
     * \param peel_layers Layer limit for depth peeling; zero requests all layers.
     * \return Color, depth, and picking buffers for the rendered frame.
     */
            virtual FrameImage render(std::uint32_t width,
                                      std::uint32_t height,
                                      const std::vector<DrawCommand> &commands,
                                      const vec4 &background = vec4(1, 1, 1, 1),
                                      unsigned samples = 1,
                                      TransparencyMethod transparency = TransparencyMethod::sorted,
                                      unsigned peel_layers = 4) = 0;
            // Render scene IDs and depth at one top-left framebuffer pixel center.
            // The full viewport and draw order are preserved. Overlays are excluded.
            /**
     * \brief Sample scene depth and IDs at a top-left framebuffer pixel.
     * \param width Full render-target width in pixels.
     * \param height Full render-target height in pixels.
     * \param commands Draw commands for the frame.
     * \param x Pixel column.
     * \param y Pixel row from the top.
     * \return Scene ID and depth at the pixel center.
     */
            virtual ScenePixel sample_pixel(std::uint32_t width,
                                            std::uint32_t height,
                                            const std::vector<DrawCommand> &commands,
                                            std::uint32_t x,
                                            std::uint32_t y) = 0;
            // Queue a frame for a configured presentation surface, without CPU readback.
            // False means zero extent/out-of-date surface; the host should retry after events.
            // Headless devices report an unsupported operation explicitly.
            /**
     * \brief Queue commands for presentation to a configured surface.
     * \param width Surface width in pixels.
     * \param height Surface height in pixels.
     * \param commands Draw commands for the frame.
     * \param background RGBA clear color.
     * \param samples Requested MSAA sample count.
     * \param transparency Method for transparent commands.
     * \param peel_layers Layer limit for depth peeling; zero requests all layers.
     * \return False for a zero extent or out-of-date surface; unsupported
     *         headless presentation reports an error.
     */
            virtual bool present(std::uint32_t width,
                                 std::uint32_t height,
                                 const std::vector<DrawCommand> &commands,
                                 const vec4 &background = vec4(1, 1, 1, 1),
                                 unsigned samples = 1,
                                 TransparencyMethod transparency = TransparencyMethod::sorted,
                                 unsigned peel_layers = 4) = 0;
            /// Return the number of validation errors reported by the backend.
            /// \return Cumulative backend validation-error count.
            virtual unsigned validation_errors() const = 0;
            // Synchronous GPU polygon upload, compute dispatch and selection readback.
            // Externally synchronized like render(); no render targets are modified.
            /**
     * \brief Select point source slots inside a framebuffer polygon.
     * \param request Point data, polygon, bitset, and transform for selection.
     * \return Updated selection bitset.
     */
            virtual std::vector<std::uint32_t> select_points(const PointSelectionRequest &request) = 0;
            // Cumulative position-resource realizations, useful for transfer diagnostics.
            // Rendering and selection share these uploads; attribute uploads are separate.
            /// Return cumulative point-position resource realizations.
            /// \return Cumulative number of point-position resource realizations.
            virtual std::uint64_t point_upload_count() const = 0;
            /// Return transparent fragments dropped by linked-list OIT pool overflow in
            /// the most recent frame. Zero unless that frame used
            /// TransparencyMethod::linked_lists and exhausted its node pool; raise the
            /// per-pixel fragment capacity (peel_layers) to reduce dropping.
            /// \return Number of fragments dropped by linked-list OIT overflow last frame.
            virtual std::uint32_t oit_dropped_fragments() const = 0;
        };

        // Implemented by the renderer Vulkan backend. Throws if devices or requested
        // validation facilities are unavailable. Sync validation implies validation;
        // device_name optionally filters physical devices by name substring.
        // use_native_large_points controls the optional largePoints feature; false forces
        // portable expanded-point rendering.
        std::unique_ptr<RenderDevice> create_vulkan_device(bool validation = false,
                                                           bool sync_validation = false,
                                                           const std::string &device_name = {});
        std::unique_ptr<RenderDevice> create_vulkan_device(bool validation,
                                                           bool sync_validation,
                                                           const std::string &device_name,
                                                           bool use_native_large_points);

    } // namespace graphics
} // namespace easy3d
#endif
