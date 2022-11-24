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


#include "widget_checker_sphere.h"

#include <QMouseEvent>
#include <QOpenGLFunctions>

#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
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
    delete checkerSphere_;
    delete lightSphere_;
    delete camera_;
    doneCurrent();
}


void WidgetCheckerSphere::initializeGL()
{
    QOpenGLWidget::initializeGL();

    QOpenGLFunctions* func = context()->functions();
    func->initializeOpenGLFunctions();
    OpenglUtil::init();

    func->glEnable(GL_DEPTH_TEST);
    func->glClearDepthf(1.0f);
    func->glClearColor(
                static_cast<float>(backgroundColor_.redF()),
                static_cast<float>(backgroundColor_.greenF()),
                static_cast<float>(backgroundColor_.blueF()),
                1.0f
            );

    camera_ = new Camera;
    camera_->setType(Camera::ORTHOGRAPHIC);
    camera_->setScreenWidthAndHeight(width(), height());
    camera_->setPosition(vec3(0.0f, 0.0f, 1.0f));
    // either set view direction
    //camera_->setViewDirection(vec3(0.0, 0.0, -1.0));
    // or set lookAt point
    camera_->lookAt(vec3(0.0f, 0.0f, 0.0f));
    // allow some margin for the light sphere to be visible near the border of the viewer
    camera_->setSceneRadius(1.1f);
    camera_->showEntireScene();
}


void WidgetCheckerSphere::resizeGL(int w, int h)
{
    QOpenGLWidget::resizeGL(w, h);

    // The viewport is set up by QOpenGLWidget before drawing.
    // So I don't need to set up.
    // glViewport(0, 0, static_cast<int>(w * highdpi_), static_cast<int>(h * highdpi_));

    camera_->setScreenWidthAndHeight(w, h);
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
    update();
    emit lightPositionChanged();
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

    checkerSphere_ = new TrianglesDrawable("checker_sphere");
    checkerSphere_->update_vertex_buffer(vertices);
    checkerSphere_->update_normal_buffer(normals);
    checkerSphere_->update_color_buffer(colors);

    // light
    vertices.clear();
    normals.clear();
    colors.clear();
    shape::create_sphere(vec3(0, 0, 0), 0.1, 10, 10, vec3(1, 0, 0), vertices, normals, colors);
    lightSphere_ = new TrianglesDrawable("light");
    lightSphere_->update_vertex_buffer(vertices);
    lightSphere_->update_normal_buffer(normals);
    lightSphere_->update_color_buffer(colors);
}


void WidgetCheckerSphere::paintGL() {
    if (!checkerSphere_)
        createSpheres();

    ShaderProgram* program = ShaderManager::get_program("surface/surface");
    if (!program) {
        std::vector<ShaderProgram::Attribute> attributes;
        attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
        attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"));
        attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
        attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
        program = ShaderManager::create_program_from_files("surface/surface", attributes);
    }
    if (!program)
        return;

    const mat4& MVP = camera_->modelViewProjectionMatrix();
    // camera position is defined in world coordinate system.
    const vec3& wCamPos = camera_->position();
    // it can also be computed as follows:
    //const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
    const mat4& MV = camera_->modelViewMatrix();
    const vec4& wLightPos = inverse(MV) * setting::light_position;

    program->bind();
    program->set_uniform("MVP", MVP)
            ->set_uniform("NORMAL", mat3::identity())   // needs be padded when using uniform blocks
            ->set_uniform("lighting", true)
            ->set_uniform("wLightPos", wLightPos)
            ->set_uniform("wCamPos", wCamPos)
            ->set_uniform("two_sides_lighting", false)
            ->set_uniform("distinct_back_color", false)
            ->set_uniform("smooth_shading", true)
            ->set_uniform("ssaoEnabled", false)
            ->set_uniform("per_vertex_color", true)
            ->set_block_uniform("Material", "ambient", setting::material_ambient)
            ->set_block_uniform("Material", "specular", setting::material_specular)
            ->set_block_uniform("Material", "shininess", &setting::material_shininess)
            ->set_uniform("highlight", false)
            ->set_uniform("selected", false)
            ->set_uniform("highlight_color", setting::highlight_color)
            ->set_uniform("clippingPlaneEnabled", false)
            ->set_uniform("use_texture", false);

    program->set_uniform("MANIP", mat4::translation(lightPos_.xyz()));
    lightSphere_->gl_draw();

    program->set_uniform("MANIP", mat4::identity());
    checkerSphere_->gl_draw();

    program->release();
}
