// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_OFFSCREEN_H
#define EASY3D_VIEWER_OFFSCREEN_H

#include <easy3d/viewer/viewer.h>

#ifndef EASY3D_HAS_VULKAN   // the Vulkan counterpart is not folded in yet (work item U1)
namespace easy3d {
// OpenGL offscreen counterpart. Owns a hidden shared context; dimensions are
// framebuffer pixels, independent of the hidden window's display scale.
class OffScreen : public Viewer {
public:
    explicit OffScreen(unsigned int width = 800, unsigned int height = 600, const OpenGLOptions& gl_options = {});
    ~OffScreen() override;
    bool render(const std::string& filename, float scaling = 1, int samples = 4,
                int background = 1, bool expand = true) const;
    void resize(int width, int height) override;
    void framebuffer_size(int& width, int& height) const override;
    int framebuffer_width() const override { return width(); }
    int framebuffer_height() const override { return height(); }
    Model* add_model(const std::string& file, bool create = true) override;
    Model* add_model(std::shared_ptr<Model> model, bool create = true) override;
    using Viewer::add_model;
    Drawable* add_drawable(std::shared_ptr<Drawable> drawable) override;
    using Viewer::add_drawable;
private:
    void activate() const;
};
}
#endif

#endif	// EASY3D_VIEWER_OFFSCREEN_H
