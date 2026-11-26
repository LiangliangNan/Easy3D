/** \cond LICENSE
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
 \endcond */


#include "widget_checker_sphere.h"

#include <QMouseEvent>
#include <QOpenGLFunctions>

#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_renderer.h>
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/opengl_util.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/util/setting.h>


using namespace easy3d;


WidgetCheckerSphere::WidgetCheckerSphere(QWidget *parent)
    : QOpenGLWidget(parent)
    , camera_(nullptr)
    , checkerSphere_(nullptr)
    , lightSphere_(nullptr)
    , mouseDown_(false)
    , scene_radius_(1.1f)
{
    defaultLightPos_ = setting::light_position;
    defaultLightPos_ = normalize(defaultLightPos_);
    lightPos_ = defaultLightPos_;

    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}


WidgetCheckerSphere::~WidgetCheckerSphere()
{
    makeCurrent();
    device_.reset();
    drawable_renderer_.reset();
    checkerSphere_.reset();
    lightSphere_.reset();
    delete camera_;
    doneCurrent();
}


void WidgetCheckerSphere::initializeGL()
{
    QOpenGLWidget::initializeGL();
    QOpenGLFunctions* func = context()->functions();
    func->initializeOpenGLFunctions();
    OpenglUtil::init();
    drawable_renderer_.reset(new graphics::DrawableRenderer);
    device_ = graphics::create_device();

    createSpheres();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearDepth(1.0f);
	glClearColor(
		backgroundColor_.redF(),
		backgroundColor_.greenF(),
		backgroundColor_.blueF(),
		1.0f
	);

    camera_ = new Camera;
    camera_->set_type(Camera::ORTHOGRAPHIC);
    camera_->set_screen_width_and_height(width(), height());
    camera_->set_position(vec3(0.0f, 0.0f, 1.0f));
    // either set view direction
    //camera_->set_view_direction(vec3(0.0, 0.0, -1.0));
    // or set the look_at() point
    camera_->look_at(vec3(0.0f, 0.0f, 0.0f));
    // allow some margin for the light sphere to be visible near the border of the viewer
    camera_->set_scene_radius(1.1f);
    camera_->show_entire_scene();
}


void WidgetCheckerSphere::resizeGL(int w, int h)
{
    QOpenGLWidget::resizeGL(w, h);

    // The viewport is set up by QOpenGLWidget before drawing.
    // So I don't need to set up.
    // glViewport(0, 0, static_cast<int>(w * highdpi_), static_cast<int>(h * highdpi_));

    camera_->set_screen_width_and_height(w, h);
}


void WidgetCheckerSphere::mousePressEvent(QMouseEvent * e) {
    mouseDown_ = true;
    const vec3& p = projectToSphere(e->pos());

    // set for light position hint on the sphere
    lightPos_ = vec4(p.x, p.y, p.z, 0);
    updateLighting();
}


void WidgetCheckerSphere::mouseMoveEvent(QMouseEvent * e) {
    if (mouseDown_) {
        const vec3& p = projectToSphere(e->pos());

        // set for light position hint on the sphere
        lightPos_ = vec4(p.x, p.y, p.z, 0);
        updateLighting();
    }
}


void WidgetCheckerSphere::mouseReleaseEvent(QMouseEvent * e) {
    mouseDown_ = false;
}


void WidgetCheckerSphere::resetLightPosition() {
    // set for light position hint on the sphere
    lightPos_ = defaultLightPos_;
    updateLighting();
}


vec3 WidgetCheckerSphere::projectToSphere(const QPoint& p)
{
    float x = p.x() * (scene_radius_ * 2.0f / width()) - scene_radius_;
    float y = scene_radius_ - p.y() * (scene_radius_ * 2.0f / height());
    float z = 0.0f;
    float sqr_len = x*x + y*y;
    if (sqr_len>1) {
        // if the projection point is not over the sphere, then normalize it
        float norm = 1.0f/std::sqrt(sqr_len);
        x *= norm;
        y *= norm;
        z  = 0.0f;
    }
    else { // computing the z coordinate of projected point
        z = std::sqrt(1.0f-sqr_len);
    }
    return vec3(x, y, z);
}


void WidgetCheckerSphere::updateLighting() {
    setting::light_position = lightPos_;
    emit lightPositionChanged();
    update();
}


void WidgetCheckerSphere::createSpheres() {
	const vec3 center(0, 0, 0);
    const double radius = 1.0;
	const int checker_size = 5;
    const int slices = 50;
    const int stacks = 40;
    const vec3 color1(0.6f, 0.6f, 0.6f);
    const vec3 color2(1, 1, 1);

    std::vector<vec3> vertices, normals, colors;
	shape::create_checker_sphere(center, radius, slices, stacks, checker_size, color1, color2, vertices, normals, colors);

    checkerSphere_ = std::make_shared<TrianglesDrawable>("checker_sphere");
    checkerSphere_->update_vertex_buffer(vertices);
    checkerSphere_->update_normal_buffer(normals);
    checkerSphere_->update_color_buffer(colors);
    checkerSphere_->set_property_coloring(State::VERTEX);
    checkerSphere_->set_lighting(true);

    // light
    vertices.clear();
    normals.clear();
    colors.clear();
    shape::create_sphere(vec3(0, 0, 0), 0.1, 10, 10, vec3(1, 0, 0), vertices, normals, colors);
    lightSphere_ = std::make_shared<TrianglesDrawable>("light");
    lightSphere_->update_vertex_buffer(vertices);
    lightSphere_->update_normal_buffer(normals);
    lightSphere_->update_color_buffer(colors);
    lightSphere_->set_property_coloring(State::VERTEX);
    lightSphere_->set_lighting(true);
}


void WidgetCheckerSphere::paintGL() {
    if (!checkerSphere_ || !device_ || !drawable_renderer_)
        return;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    std::vector<graphics::DrawCommand> commands;
    commands.push_back(drawable_renderer_->command(checkerSphere_, 0, *camera_, {}));
    auto light = drawable_renderer_->command(lightSphere_, 1, *camera_, {});
    const mat4 manip = mat4::translation(lightPos_.xyz());
    light.model = manip;
    light.model_view = camera_->model_view_matrix() * manip;
    light.model_view_projection = camera_->model_view_projection_matrix() * manip;
    light.normal_matrix = mat3::identity();
    commands.push_back(std::move(light));

    GLint samples = 1;
    glGetIntegerv(GL_SAMPLES, &samples);
    const vec4 background(backgroundColor_.redF(), backgroundColor_.greenF(), backgroundColor_.blueF(), 1.0f);
    device_->present(uint32_t(viewport[2]), uint32_t(viewport[3]), commands, background, uint32_t(samples));
}
