// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/opengl.h>
#endif
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "easy3d/viewer/viewer.h"
using namespace easy3d;
namespace {
void require(bool value, const char *message) {
    if (!value)
        throw std::runtime_error(message);
}
bool near(const vec3 &a, const vec3 &b, float epsilon = 1e-4f) { return distance(a, b) < epsilon; }
std::shared_ptr<ViewerBase> make_host(unsigned samples, bool validation) {
#ifdef EASY3D_HAS_VULKAN
    {
        VulkanViewerOptions o;
        o.width = 181;
        o.height = 151;
        o.visible = false;
        o.samples = samples;
        o.device.validation = validation;
        o.device.synchronization_validation = validation;
        auto v = std::make_shared<VulkanViewer>("Portable camera queries", o);
        v->set_usage("");
        v->camera()->setViewDirection(vec3(0, 0, -1));
        v->camera()->setUpVector(vec3(0, 1, 0));
        return v;
    }
#else
    auto v = std::make_shared<OpenGLViewer>("Portable camera queries", int(samples), 3, 2, false, true, 24, 8, 181, 151);
    v->hide();
    v->set_usage("");
    v->camera()->setViewDirection(vec3(0, 0, -1));
    v->camera()->setUpVector(vec3(0, 1, 0));
    return v;
#endif
}

void check(unsigned samples, bool validation) {
    auto v = make_host(samples, validation);
    auto front = std::make_shared<TrianglesDrawable>("front");
    v->add_drawable(front);
    front->update_vertex_buffer({vec3(-1, -1, .2f), vec3(1, -1, .2f), vec3(0, 1, .2f)});
    front->set_lighting(false);
    front->set_uniform_coloring(vec4(.8f, .2f, .1f, 1));
    auto back = std::make_shared<TrianglesDrawable>("back");
    v->add_drawable(back);
    back->update_vertex_buffer({vec3(-1, -1, -.3f), vec3(1, -1, -.3f), vec3(0, 1, -.3f)});
    back->set_lighting(false);
    back->set_uniform_coloring(vec4(0, 0, 1, 1));
    v->fit_screen();
    unsigned frames = 0;
    ViewerCallbacks callbacks;
    callbacks.pre_draw = [&](ViewerBase *) { ++frames; };
    callbacks.post_draw = [&](ViewerBase *) { ++frames; };
    v->set_callbacks(callbacks);
    for (auto type : {Camera::PERSPECTIVE, Camera::ORTHOGRAPHIC}) {
        v->camera()->setType(type);
        for (auto p : {vec3(.1f, .15f, .2f), vec3(-.2f, -.3f, -.3f), vec3(10, 10, 0)})
            require(near(v->unproject_point(v->project_point(p)), p, 2e-3f), "projection round trip failed");
        int fw, fh;
        v->framebuffer_size(fw, fh);
        const int x = v->width() / 2, y = v->height() / 2;
        const mat4 original = v->camera()->modelViewProjectionMatrix();
#ifndef EASY3D_HAS_VULKAN
        GLuint ubo = 0, pbo = 0;
        {
            glfwMakeContextCurrent(v->window());
            glGenBuffers(1, &ubo);
            glBindBuffer(GL_UNIFORM_BUFFER, ubo);
            glBufferData(GL_UNIFORM_BUFFER, 256, nullptr, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);
            glGenBuffers(1, &pbo);
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
            glBufferData(GL_PIXEL_PACK_BUFFER, 1024, nullptr, GL_DYNAMIC_READ);
            glPixelStorei(GL_PACK_ROW_LENGTH, 7);
            glPixelStorei(GL_PACK_SKIP_PIXELS, 2);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glEnable(GL_PRIMITIVE_RESTART);
            glPrimitiveRestartIndex(1);
            glEnable((GL_CLIP_DISTANCE0 + 7));
            glEnable(GL_SCISSOR_TEST);
            glScissor(0, 0, 1, 1);
            glDepthMask(GL_FALSE);
            glViewport(3, 4, 19, 23);
        }
#endif
        // Deliberately query before the first frame, with hostile GL state.
        bool found = false;
        const auto hit = v->point_under_pixel(x, y, found);
        require(found && std::abs(hit.z - .2f) < 2e-4f, "fresh query lost foreground depth");
        const auto projected = v->project_point(hit);
        const float expected_x = float((std::floor(double(x) * fw / v->width()) + .5) * v->width() / fw);
        const float expected_y = float((std::floor(double(y) * fh / v->height()) + .5) * v->height() / fh);
        require(std::abs(projected.x - expected_x) < 1e-3f && std::abs(projected.y - expected_y) < 1e-3f,
                "query missed HiDPI framebuffer pixel center");
        require(frames == 0, "query ran frame callbacks");
        for (unsigned i = 0; i < 16; ++i)
            require(original[i] == v->camera()->modelViewProjectionMatrix()[i], "query changed camera matrices");
#ifndef EASY3D_HAS_VULKAN
        {
            GLint binding = 0, viewport[4], pack = 0;
            GLboolean mask;
            glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, 1, &binding);
            require(GLuint(binding) == ubo, "query leaked indexed uniform binding");
            glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &binding);
            require(GLuint(binding) == pbo, "query leaked pixel pack buffer");
            glGetIntegerv(GL_PACK_ROW_LENGTH, &pack);
            require(pack == 7, "query leaked pixel pack layout");
            glGetIntegerv(GL_VIEWPORT, viewport);
            glGetBooleanv(GL_DEPTH_WRITEMASK, &mask);
            require(viewport[0] == 3 && viewport[1] == 4 && viewport[2] == 19 && viewport[3] == 23 && !mask &&
                        glIsEnabled(GL_SCISSOR_TEST),
                    "query leaked pipeline state");
            GLint polygon[2];
            glGetIntegerv(GL_POLYGON_MODE, polygon);
            require(polygon[0] == GL_LINE && glIsEnabled(GL_PRIMITIVE_RESTART) && glIsEnabled((GL_CLIP_DISTANCE0 + 7)),
                    "query leaked rasterization state");
            require(glGetError() == GL_NO_ERROR, "query produced OpenGL errors");
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDisable(GL_PRIMITIVE_RESTART);
            glDisable((GL_CLIP_DISTANCE0 + 7));
            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            glDeleteBuffers(1, &pbo);
            glDeleteBuffers(1, &ubo);
            glPixelStorei(GL_PACK_ROW_LENGTH, 0);
            glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
            glDepthMask(GL_TRUE);
            glDisable(GL_SCISSOR_TEST);
            glViewport(0, 0, fw, fh);
        }
#endif
        front->set_visible(false);
        const auto far = v->point_under_pixel(x, y, found);
        require(found && std::abs(far.z + .3f) < 2e-4f, "query ignored drawable visibility");
        front->set_visible(true);
        v->point_under_pixel(0, 0, found);
        require(!found, "background returned a point");
        v->point_under_pixel(-1, y, found);
        require(!found, "negative coordinate returned a point");
        v->point_under_pixel(v->width(), y, found);
        require(!found, "outside coordinate returned a point");
        auto sphere = std::make_shared<PointsDrawable>("sphere");
        v->add_drawable(sphere);
        sphere->update_vertex_buffer({vec3(0, 0, .5f)});
        sphere->set_impostor_type(PointsDrawable::SPHERE);
        sphere->set_point_size(24);
        const auto surface = v->point_under_pixel(x, y, found);
        require(found && surface.z > .5f, "sphere query lost analytic surface depth");
        sphere->update_vertex_buffer({vec3(3, 3, .5f)});
        require(near(v->point_under_pixel(x, y, found), hit) && found, "query used stale point geometry");
        v->delete_drawable(sphere.get());
        auto hud = std::make_shared<LinesDrawable2D>("hud");
        v->add_drawable(hud);
        hud->update_vertex_buffer({vec2(0, 0), vec2(float(v->width()), float(v->height()))}, v->width(), v->height());
        require(near(v->point_under_pixel(x, y, found), hit) && found, "HUD obscured scene query");
        v->delete_drawable(hud.get());
    }
    // Custom matrices must survive the temporary framebuffer-sized Camera copy.
    const auto projection = v->camera()->projectionMatrix();
    auto shifted = projection;
    shifted(0, 3) += .15f;
    shifted(1, 3) -= .1f;
    v->camera()->set_projection_matrix(shifted);
    bool custom_found = false;
    const auto custom_hit = v->point_under_pixel(v->width() / 2, v->height() / 2, custom_found);
    require(custom_found && std::abs(custom_hit.z - .2f) < 2e-4f, "query discarded custom camera matrices");
    const auto custom_window = v->project_point(custom_hit);
    require(std::abs(custom_window.x - v->width() / 2) < 1 && std::abs(custom_window.y - v->height() / 2) < 1,
            "query unprojected a different projection");
    v->camera()->set_projection_matrix(mat4(0.0f));
    bool singular = false;
    try {
        v->unproject_point(vec3(0));
    } catch (const std::invalid_argument &) {
        singular = true;
    }
    require(singular, "singular camera accepted");
    v->camera()->set_projection_matrix(projection);
    glfwSetWindowSize(v->window(), 203, 163);
    v->poll_events();
    bool resized = false;
    const auto resized_hit = v->point_under_pixel(v->width() / 2, v->height() / 2, resized);
    require(resized && std::abs(resized_hit.z - .2f) < 2e-4f, "query failed after logical/framebuffer resize");
#ifndef EASY3D_HAS_VULKAN
    {
        auto other = std::make_shared<OpenGLViewer>("Portable camera queries other", 4, 3, 2, false, true, 24, 8, 181, 151);
        other->hide();
        glfwMakeContextCurrent(other->window());
        bool restored = false;
        v->point_under_pixel(v->width() / 2, v->height() / 2, restored);
        require(restored && glfwGetCurrentContext() == other->window(),
                "query failed to restore another current context");
        require(other->draw_frame(), "query broke another shared viewer");
    }
#endif
    require(v->snapshot("portable-query.png", 1, 4, 0), "snapshot after query failed");
    int w, h, c;
    std::vector<unsigned char> rgba;
    require(ImageIO::load("portable-query.png", rgba, w, h, c, 4, false), "query snapshot missing");
    const size_t center = (size_t(h / 2) * w + w / 2) * 4;
    require(std::abs(int(rgba[center]) - 204) <= 2 && std::abs(int(rgba[center + 1]) - 51) <= 2,
            "query damaged subsequent shader bindings/depth");
    require(frames == 0 && v->draw_frame() && frames == 2, "query advanced frame state or broke later rendering");
    bool rejected = false;
    try {
        v->project_point(vec3(std::numeric_limits<float>::quiet_NaN(), 0, 0));
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    require(rejected, "nonfinite projection accepted");
    v->clear_scene();
    bool found = true;
    v->point_under_pixel(v->width() / 2, v->height() / 2, found);
    require(!found, "cleared scene returned stale depth");
    auto *plane = ClippingPlane::instance();
    const bool enabled = plane->is_enabled(), visible = plane->is_visible();
    plane->fit_scene(vec3(0), 1);
    plane->set_enabled(true);
    plane->set_visible(true);
    v->point_under_pixel(v->width() / 2, v->height() / 2, found);
    plane->set_enabled(enabled);
    plane->set_visible(visible);
    require(!found, "clipping-plane guide returned a scene point");
    require(v->validation_errors() == 0, "query reported Vulkan validation errors");
}
} // namespace
int main(int argc, char **argv) {
    try {
        initialize(false, true, false);
        bool validation = false;
        for (int i = 1; i < argc; ++i)
            if (std::string(argv[i]) == "--validation")
                validation = true;
        check(1, validation);
        check(4, validation);
        std::cout << "Portable viewer camera queries passed\n";
        return 0;
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
