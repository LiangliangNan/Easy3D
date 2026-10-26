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
// Opaque uniform/property-colored triangles and plain one-pixel points/lines only.
// Unsupported effects/textures/impostors fail explicitly; see docs/vulkan.md.
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
    bool delete_model(Model* model);
    const std::vector<std::shared_ptr<Model>>& models() const;
    Model* current_model() const;
    Drawable* add_drawable(std::shared_ptr<Drawable> drawable);
    bool delete_drawable(Drawable* drawable);
    const std::vector<std::shared_ptr<Drawable>>& drawables() const;
    void clear_scene();
    void fit_screen();
    graphics::FrameImage capture() const;
    // Single sample only at this milestone. Other sample counts are rejected.
    bool render(const std::string& filename, float scaling = 1.0f, int samples = 1,
                int back_ground = 1, bool expand = true) const;
    const graphics::RenderDevice& device() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
#endif
