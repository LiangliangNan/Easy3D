// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_VULKAN_OFFSCREEN_H
#define EASY3D_VIEWER_VULKAN_OFFSCREEN_H
#include <easy3d/graphics/render_device.h>
#include <easy3d/renderer/camera.h>
#include <memory>
#include <string>
#include <vector>

namespace easy3d {
class Model;
class Drawable;

// Experimental native Vulkan counterpart of OffScreen. Reuses Camera, Model,
// Renderer and Drawable APIs without creating an OpenGL context or GLFW window.
// The shared renderer library supplies CPU scene policies; its OpenGL backend
// can be disabled completely, including in builds of this adapter.
// Supports the shared geometry, texture and impostor subset documented in docs/vulkan.md.
// Unsupported scene effects fail explicitly.
class VulkanOffScreen {
public:
    explicit VulkanOffScreen(int width = 800, int height = 600,
                            const graphics::DeviceOptions& options = {});
    virtual ~VulkanOffScreen();
    VulkanOffScreen(const VulkanOffScreen&) = delete;
    VulkanOffScreen& operator=(const VulkanOffScreen&) = delete;
    Camera* camera();
    const Camera* camera() const;
    int width() const;
    int height() const;
    void resize(int width, int height);
    void set_background_color(const vec4& color);
    const vec4& background_color() const;
    virtual Model* add_model(const std::string& filename, bool create_default_drawables = true);
    Model* add_model(std::shared_ptr<Model> model, bool create_default_drawables = true);
    Model* add_model(Model* model, bool create_default_drawables = true);
    bool delete_model(Model* model);
    const std::vector<std::shared_ptr<Model>>& models() const;
    Model* current_model() const;
    Drawable* add_drawable(std::shared_ptr<Drawable> drawable);
    bool delete_drawable(Drawable* drawable);
    const std::vector<std::shared_ptr<Drawable>>& drawables() const;
    void clear_scene();
    // Triangle annotations and HUD overlays. Their immutable
    // geometry/images are retained; MVPs are supplied by the caller (for example
    // graphics::screen_space_projection()). overlay_depth_test annotations test
    // scene depth and blend first, in input order; HUD overlays follow, in order.
    // Sort overlapping depth-tested annotations back-to-front. None write depth/IDs.
    void set_overlays(std::vector<graphics::DrawCommand> commands);
    void fit_screen(const Model* model = nullptr);
    // Per-viewer world-space halfspaces, keeping dot(plane,vec4(world,1)) >= 0.
    // Empty disables clipping. At most two finite, nonzero-normal planes.
    // Overlays remain unclipped. Scene clearing retains this configuration.
    void set_clipping_planes(const std::vector<vec4>& planes);
    const std::vector<vec4>& clipping_planes() const;
    struct PickResult {
        std::shared_ptr<Model> model; // null for a standalone drawable
        std::shared_ptr<Drawable> drawable;
        std::uint32_t primitive = UINT32_MAX; // generated primitive within this drawable
        int face_index = -1; // original SurfaceMesh face, when provenance is available
        vec3 position = vec3(0); // world-space surface point at the pixel center
        float depth = 1;
        explicit operator bool() const { return bool(drawable); }
    };
    // Synchronous visible-surface picking, in top-left framebuffer pixels. Returns
    // an empty result for background/outside pixels. Retains source ownership;
    // face indices describe the current mesh, not future topology edits.
    PickResult pick(int x, int y) const;
    // Reconstruct a top-left framebuffer pixel center from [0,1] readback depth.
    // Unlike Camera's legacy coordinate helper, this uses continuous framebuffer coordinates.
    vec3 unproject_pixel(int x, int y, float depth) const;
    // Color is multisampled when requested; IDs/depth stay center-sampled.
    // Invalid/unsupported requests throw, allowing callers/tests to handle them.
    graphics::FrameImage capture(unsigned samples = 1) const;
    bool render(const std::string& filename, float scaling = 1.0f, int samples = 1,
                int back_ground = 1, bool expand = true) const;
    const graphics::RenderDevice& device() const;
protected:
    VulkanOffScreen(int width, int height, std::unique_ptr<graphics::RenderDevice> device);
    bool present(unsigned samples = 1);
    // Window hosts must destroy the Vulkan surface/device before the native window.
    void release_device();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
#endif
