// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_VIEWER_OPENGL_OFFSCREEN_H
#define EASY3D_VIEWER_OPENGL_OFFSCREEN_H
#include <easy3d/viewer/opengl_viewer.h>
namespace easy3d {
// OpenGL offscreen counterpart. Owns a hidden shared context; dimensions are
// framebuffer pixels, independent of the hidden window's display scale.
class OpenGLOffScreen : public OpenGLViewer {
public:
    explicit OpenGLOffScreen(int width = 800, int height = 600);
    ~OpenGLOffScreen() override;
    bool render(const std::string& filename, float scaling = 1, int samples = 4,
                int background = 1, bool expand = true) const;
    void resize(int width, int height) override;
    void framebuffer_size(int& width, int& height) const override;
    int framebuffer_width() const override { return width(); }
    int framebuffer_height() const override { return height(); }
    Model* add_model(const std::string& file, bool create = true) override;
    Model* add_model(std::shared_ptr<Model> model, bool create = true) override;
    using OpenGLViewer::add_model;
    Drawable* add_drawable(std::shared_ptr<Drawable> drawable) override;
    using OpenGLViewer::add_drawable;
private:
    void activate() const;
};
}
#endif
