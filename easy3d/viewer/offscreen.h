// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_OFFSCREEN_H
#define EASY3D_VIEWER_OFFSCREEN_H
#include <easy3d/viewer/viewer.h>
namespace easy3d {
class OffScreen : protected Viewer {
public:
    // ... some public functions: the unified APIs used by both backends
    explicit OffScreen(int width = 800, int height = 600,
#ifdef EASY3D_HAS_VULKAN
                       const VulkanOptions& options = {});
#else
                       const OpenGLOptions& options = {});
#endif
    ~OffScreen() override;
    bool render(const std::string& filename, float scaling = 1, int samples = 4,
                int background = 1, bool expand = true) const;
    void resize(int width, int height) override;
    void framebuffer_size(int& width, int& height) const override;
    int framebuffer_width() const override { return width(); }
    int framebuffer_height() const override { return height(); }
    Model* add_model(const std::string& file, bool create = true) override;
    Model* add_model(std::shared_ptr<Model> model, bool create = true) override;
    Drawable* add_drawable(std::shared_ptr<Drawable> drawable) override;
    using Viewer::camera;
    using Viewer::width;
    using Viewer::height;
    using Viewer::set_background_color;
    using Viewer::background_color;
    using Viewer::add_model;
    using Viewer::delete_model;
    using Viewer::models;
    using Viewer::current_model;
    using Viewer::add_drawable;
    using Viewer::delete_drawable;
    using Viewer::drawables;
    using Viewer::clear_scene;
    using Viewer::fit_screen;
#ifdef EASY3D_HAS_VULKAN
    // Transitional native scene operations, retained until the picker migration.
    using Viewer::PickResult;
    using Viewer::pick;
    using Viewer::select_points;
    using Viewer::point_resource;
    using Viewer::unproject_pixel;
    using Viewer::scan_visible_points;
    using Viewer::capture;
    using Viewer::device;
    using Viewer::set_overlays;
    using Viewer::set_clipping_planes;
    using Viewer::clipping_planes;
#endif
private:
#ifdef EASY3D_HAS_VULKAN
    // Keep this part of code minimum
#else
    // Keep this part of code minimum
    void activate() const;
#endif
};
}
#endif
