/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#include "viewer.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/primitives.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/renderer/dual_depth_peeling.h>
#include <easy3d/renderer/eye_dome_lighting.h>
#include <easy3d/renderer/read_pixel.h>
#include <easy3d/renderer/opengl_info.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/setting.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/text_renderer.h>
#include <easy3d/renderer/buffers.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/opengl_timer.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/file_system.h>

#include <QKeyEvent>
#include <QPainter>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QApplication>
#include <QClipboard>


using namespace easy3d;


ViewerQt::ViewerQt(QWidget *parent /* = nullptr*/)
        : QOpenGLWidget(parent), func_(nullptr), gpu_timer_(nullptr), texter_(nullptr), pressed_button_(Qt::NoButton),
          mouse_pressed_pos_(0, 0), mouse_previous_pos_(0, 0), show_pivot_point_(false), drawable_axes_(nullptr),
          model_idx_(-1) {
    // like Qt::StrongFocus plus the widget accepts focus by using the mouse wheel.
    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);

    camera_ = new Camera;
    camera_->setType(Camera::PERSPECTIVE);
    camera_->setUpVector(vec3(0, 0, 1)); // Z pointing up
    camera_->setViewDirection(vec3(-1, 0, 0)); // X pointing out
    camera_->showEntireScene();

    easy3d::connect(&camera_->frame_modified, this, static_cast<void (ViewerQt::*)(void)>(&ViewerQt::update));
}


ViewerQt::~ViewerQt() {
    // Make sure the context is current and then explicitly
    // destroy all underlying OpenGL resources.
    makeCurrent();

    cleanup();

    doneCurrent();
}


void ViewerQt::cleanup() {
    delete camera_;
    delete drawable_axes_;
    delete gpu_timer_;
    delete texter_;

    for (auto m : models_) {
        delete m->renderer();
        delete m;
    }

    ShaderManager::terminate();
    TextureManager::terminate();
}


void ViewerQt::init() {
    const std::string file_name = resource::directory() + "/data/easy3d.ply";
    auto mesh = SurfaceMeshIO::load(file_name);
    if (mesh)
        addModel(mesh);

    // We always want to look at the front of the easy3d logo.
    camera()->setViewDirection(vec3(0, 0, -1));
    camera()->setUpVector(vec3(0, 1, 0));
    fitScreen(mesh);
}


void ViewerQt::initializeGL() {
    QOpenGLWidget::initializeGL();
    func_ = context()->functions();
    func_->initializeOpenGLFunctions();

    OpenglInfo::init();
#ifndef NDEBUG
    opengl::setup_gl_debug_callback();
#endif

    if (!func_->hasOpenGLFeature(QOpenGLFunctions::Multisample))
        throw std::runtime_error("Multisample not supported on this machine!!! ViewerQt may not run properly");
    if (!func_->hasOpenGLFeature(QOpenGLFunctions::Framebuffers))
        throw std::runtime_error(
                "Framebuffer Object is not supported on this machine!!! ViewerQt may not run properly");

    background_color_ = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    func_->glEnable(GL_DEPTH_TEST);
    func_->glClearDepthf(1.0f);
    func_->glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);

    int major_requested = QSurfaceFormat::defaultFormat().majorVersion();
    int minor_requested = QSurfaceFormat::defaultFormat().minorVersion();
    LOG(INFO) << "OpenGL vendor: " << func_->glGetString(GL_VENDOR);
    LOG(INFO) << "OpenGL renderer: " << func_->glGetString(GL_RENDERER);
    LOG(INFO) << "OpenGL version requested: " << major_requested << "." << minor_requested;
    LOG(INFO) << "OpenGL version received: " << func_->glGetString(GL_VERSION);
    LOG(INFO) << "GLSL version received: " << func_->glGetString(GL_SHADING_LANGUAGE_VERSION);

    int major = 0;
    func_->glGetIntegerv(GL_MAJOR_VERSION, &major);
    int minor = 0;
    func_->glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major * 10 + minor < 32) {
        throw std::runtime_error("ViewerQt requires at least OpenGL 3.2");
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    dpi_scaling_ = devicePixelRatioF();
#else
    dpi_scaling_ = devicePixelRatio();
#endif

    // This won't work because QOpenGLWidget draws everything in framebuffer and
    // the framebuffer has not been created in the initializeGL() method. We
    // will query the actual samples in the paintGL() method.
    //int samples_received = 0;
    //func_->glgetintegerv(gl_samples, &samples_received);

    // create TextRenderer renderer and load default fonts
    texter_ = new TextRenderer(dpi_scaling());
    texter_->add_font(resource::directory() + "/fonts/en_Earth-Normal.ttf");
    texter_->add_font(resource::directory() + "/fonts/en_Roboto-Medium.ttf");

    // create a GPU timer
    gpu_timer_ = new OpenGLTimer(false);

    // Calls user defined method.
    init();

    // print usage
    std::cout << usage() << std::endl;
}


void ViewerQt::resizeGL(int w, int h) {
    QOpenGLWidget::resizeGL(w, h);

    // The viewport is set up by QOpenGLWidget before drawing.
    // So I don't need to set up.
    // func_->glViewport(0, 0, static_cast<int>(w * highdpi_), static_cast<int>(h * highdpi_));

    camera()->setScreenWidthAndHeight(w, h);
}


void ViewerQt::setBackgroundColor(const vec4 &c) {
    background_color_ = c;

    makeCurrent();
    func_->glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
    doneCurrent();
}


void ViewerQt::mousePressEvent(QMouseEvent *e) {
    pressed_button_ = e->button();
    mouse_previous_pos_ = e->pos();
    mouse_pressed_pos_ = e->pos();

    camera_->frame()->action_start();
    if (e->modifiers() == Qt::ShiftModifier) {
        if (e->button() == Qt::LeftButton) {
            bool found = false;
            const vec3 &p = pointUnderPixel(e->pos(), found);
            if (found) {
                camera()->interpolateToLookAt(p);
                camera_->setPivotPoint(p);

                // show, but hide the visual hint of pivot point after \p delay milliseconds.
                show_pivot_point_ = true;
                const int delay = 10000;
                Timer::single_shot(delay, [&]() {
                    show_pivot_point_ = false;
                    update();
                });
            } else {
                camera_->setPivotPoint(camera_->sceneCenter());
                show_pivot_point_ = false;
            }
        } else if (e->button() == Qt::RightButton) {
            camera()->interpolateToFitScene();
            camera_->setPivotPoint(camera_->sceneCenter());
            show_pivot_point_ = false;
        }
    }

    QOpenGLWidget::mousePressEvent(e);
    update();
}


void ViewerQt::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton && e->modifiers() == Qt::ControlModifier) { // ZOOM_ON_REGION
        int xmin = std::min(mouse_pressed_pos_.x(), e->pos().x());
        int xmax = std::max(mouse_pressed_pos_.x(), e->pos().x());
        int ymin = std::min(mouse_pressed_pos_.y(), e->pos().y());
        int ymax = std::max(mouse_pressed_pos_.y(), e->pos().y());
        camera_->fitScreenRegion(xmin, ymin, xmax, ymax);
    } else
        camera_->frame()->action_end();

    pressed_button_ = Qt::NoButton;
    mouse_pressed_pos_ = QPoint(0, 0);

    QOpenGLWidget::mouseReleaseEvent(e);
    update();
}


void ViewerQt::mouseMoveEvent(QMouseEvent *e) {
    int x = e->pos().x(), y = e->pos().y();
    if (x < 0 || x > width() || y < 0 || y > height()) {
        e->ignore();
        return;
    }

    if (pressed_button_ != Qt::NoButton) { // button pressed
        // Restrict the cursor to be within the client area during dragging
        if (e->modifiers() == Qt::ControlModifier) {
            // zoom on region
        } else {
            int x = e->pos().x();
            int y = e->pos().y();
            int dx = x - mouse_previous_pos_.x();
            int dy = y - mouse_previous_pos_.y();
            if (pressed_button_ == Qt::LeftButton)
                camera_->frame()->action_rotate(x, y, dx, dy, camera_, e->modifiers() == Qt::AltModifier);
            else if (pressed_button_ == Qt::RightButton)
                camera_->frame()->action_translate(x, y, dx, dy, camera_, e->modifiers() == Qt::AltModifier);
            else if (pressed_button_ == Qt::MidButton) {
                if (dy != 0)
                    camera_->frame()->action_zoom(dy > 0 ? 1 : -1, camera_);
            }
        }
    }

    mouse_previous_pos_ = e->pos();
    QOpenGLWidget::mouseMoveEvent(e);
    update();
}


void ViewerQt::mouseDoubleClickEvent(QMouseEvent *e) {
    QOpenGLWidget::mouseDoubleClickEvent(e);
    update();
}


void ViewerQt::wheelEvent(QWheelEvent *e) {
    const int delta = e->delta();
    if (delta <= -1 || delta >= 1) {
        int dy = e->delta() > 0 ? 1 : -1;
        camera_->frame()->action_zoom(dy, camera_);
    }

    QOpenGLWidget::wheelEvent(e);
    update();
}


bool ViewerQt::saveSnapshot(const QString &file_name) {
    makeCurrent();

    int w = static_cast<int>(width() * dpi_scaling());
    int h = static_cast<int>(height() * dpi_scaling());

    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    QOpenGLFramebufferObject *fbo = new QOpenGLFramebufferObject(w, h, format);
    fbo->addColorAttachment(w, h);

    fbo->bind();
    func_->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    func_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    draw();

    fbo->release();

    const QImage &image = fbo->toImage();

    // restore the clear color
    func_->glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);

    doneCurrent();

    return image.save(file_name);
}


Model *ViewerQt::currentModel() const {
    if (models_.empty())
        return nullptr;
    if (model_idx_ < models_.size())
        return models_[model_idx_];
    return nullptr;
}

void ViewerQt::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_F1 && e->modifiers() == Qt::NoModifier)
        std::cout << usage() << std::endl;
    else if (e->key() == Qt::Key_Left && e->modifiers() == Qt::KeypadModifier) {
        float angle = static_cast<float>(1 * M_PI / 180.0); // turn left, 1 degrees each step
        camera_->frame()->action_turn(angle, camera_);
    } else if (e->key() == Qt::Key_Right && e->modifiers() == Qt::KeypadModifier) {
        float angle = static_cast<float>(1 * M_PI / 180.0); // turn right, 1 degrees each step
        camera_->frame()->action_turn(-angle, camera_);
    } else if (e->key() == Qt::Key_Up && e->modifiers() == Qt::KeypadModifier) {    // move camera forward
        float step = 0.05f * camera_->sceneRadius();
        camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, 0.0, -step)));
    } else if (e->key() == Qt::Key_Down && e->modifiers() == Qt::KeypadModifier) {// move camera backward
        float step = 0.05f * camera_->sceneRadius();
        camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, 0.0, step)));
    } else if (e->key() == Qt::Key_Left &&
               e->modifiers() == (Qt::KeypadModifier | Qt::ControlModifier)) {    // move camera left
        float step = 0.05f * camera_->sceneRadius();
        camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(-step, 0.0, 0.0)));
    } else if (e->key() == Qt::Key_Right &&
               e->modifiers() == (Qt::KeypadModifier | Qt::ControlModifier)) {    // move camera right
        float step = 0.05f * camera_->sceneRadius();
        camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(step, 0.0, 0.0)));
    } else if (e->key() == Qt::Key_Up &&
               e->modifiers() == (Qt::KeypadModifier | Qt::ControlModifier)) {    // move camera up
        float step = 0.05f * camera_->sceneRadius();
        camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, step, 0.0)));
    } else if (e->key() == Qt::Key_Down &&
               e->modifiers() == (Qt::KeypadModifier | Qt::ControlModifier)) {    // move camera down
        float step = 0.05f * camera_->sceneRadius();
        camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, -step, 0.0)));
    } else if (e->key() == Qt::Key_A && e->modifiers() == Qt::NoModifier) {
        if (drawable_axes_)
            drawable_axes_->set_visible(!drawable_axes_->is_visible());
    } else if (e->key() == Qt::Key_C && e->modifiers() == Qt::NoModifier) {
        if (currentModel())
            fitScreen(currentModel());
    } else if (e->key() == Qt::Key_F && e->modifiers() == Qt::NoModifier) {
        fitScreen();
    } else if (e->key() == Qt::Key_P && e->modifiers() == Qt::NoModifier) {
        if (camera_->type() == Camera::PERSPECTIVE)
            camera_->setType(Camera::ORTHOGRAPHIC);
        else
            camera_->setType(Camera::PERSPECTIVE);
    } else if (e->key() == Qt::Key_Space && e->modifiers() == Qt::NoModifier) {
        // Aligns camera
        Frame frame;
        frame.setTranslation(camera_->pivotPoint());
        camera_->frame()->alignWithFrame(&frame, true);

        // Aligns frame
        //if (manipulatedFrame())
        //	manipulatedFrame()->alignWithFrame(camera_->frame());
    } else if (e->key() == Qt::Key_Minus && e->modifiers() == Qt::ControlModifier)
        camera_->frame()->action_zoom(-1, camera_);
    else if (e->key() == Qt::Key_Equal && e->modifiers() == Qt::ControlModifier)
        camera_->frame()->action_zoom(1, camera_);

    else if (e->key() == Qt::Key_K && e->modifiers() == Qt::AltModifier) { // add key frame
        easy3d::Frame *frame = camera()->frame();
        camera()->keyFrameInterpolator()->addKeyFrame(*frame);
        // update scene bounding box to make sure the path is within the view frustum
        float old_radius = camera()->sceneRadius();
        float candidate_radius = distance(camera()->sceneCenter(), frame->position());
        camera()->setSceneRadius(std::max(old_radius, candidate_radius));
    } else if (e->key() == Qt::Key_D && e->modifiers() == Qt::ControlModifier) { // delete path
        camera()->keyFrameInterpolator()->deletePath();

        // update scene bounding box
        Box3 box;
        for (auto m : models_)
            box.add_box(m->bounding_box());
        camera_->setSceneBoundingBox(box.min(), box.max());
    } else if (e->key() == Qt::Key_K && e->modifiers() == Qt::ControlModifier) { // play the path
        if (camera()->keyFrameInterpolator()->interpolationIsStarted())
            camera()->keyFrameInterpolator()->stopInterpolation();
        else
            camera()->keyFrameInterpolator()->startInterpolation();
    } else if (e->key() == Qt::Key_BracketLeft && e->modifiers() == Qt::NoModifier) {
        for (auto m : models_) {
            for (auto d : m->renderer()->lines_drawables()) {
                float size = d->line_width() - 1.0f;
                if (size < 1)
                    size = 1;
                d->set_line_width(size);
            }
        }
    } else if (e->key() == Qt::Key_BracketRight && e->modifiers() == Qt::NoModifier) {
        for (auto m : models_) {
            for (auto d : m->renderer()->lines_drawables()) {
                float size = d->line_width() + 1.0f;
                d->set_line_width(size);
            }
        }
    } else if (e->key() == Qt::Key_Minus && e->modifiers() == Qt::NoModifier) {
        for (auto m : models_) {
            for (auto d : m->renderer()->points_drawables()) {
                float size = d->point_size() - 1.0f;
                if (size < 1)
                    size = 1;
                d->set_point_size(size);
            }
        }
    } else if (e->key() == Qt::Key_Equal && e->modifiers() == Qt::NoModifier) {
        for (auto m : models_) {
            for (auto d : m->renderer()->points_drawables()) {
                float size = d->point_size() + 1.0f;
                d->set_point_size(size);
            }
        }
    } else if (e->key() == Qt::Key_Comma && e->modifiers() == Qt::NoModifier) {
        int pre_idx = model_idx_;
        if (models_.empty())
            model_idx_ = -1;
        else
            model_idx_ = int((model_idx_ - 1 + models_.size()) % models_.size());
        if (model_idx_ != pre_idx) {
            if (model_idx_ >= 0)
                LOG(INFO) << "current model: " << model_idx_ << ", " << models_[model_idx_]->name();
        }
    } else if (e->key() == Qt::Key_Period && e->modifiers() == Qt::NoModifier) {
        int pre_idx = model_idx_;
        if (models_.empty())
            model_idx_ = -1;
        else
            model_idx_ = int((model_idx_ + 1) % models_.size());
        if (model_idx_ != pre_idx) {
            if (model_idx_ >= 0)
                LOG(INFO) << "current model: " << model_idx_ << ", " << models_[model_idx_]->name();
        }
    } else if (e->key() == Qt::Key_Delete && e->modifiers() == Qt::NoModifier) {
        if (currentModel())
            deleteModel(currentModel());
    } else if (e->key() == Qt::Key_E && e->modifiers() == Qt::NoModifier) {
        if (currentModel()) {
            auto *edges = currentModel()->renderer()->get_lines_drawable("edges");
            if (!edges) {
                if (!dynamic_cast<PointCloud *>(currentModel())) { // no default "edges" drawable for point clouds
                    edges = currentModel()->renderer()->add_lines_drawable("edges");
                    if (dynamic_cast<SurfaceMesh *>(currentModel())) {
                        edges->set_uniform_coloring(setting::surface_mesh_edges_color);
                        edges->set_line_width(setting::surface_mesh_edges_line_width);
                    } else if (dynamic_cast<Graph *>(currentModel())) {
                        edges->set_uniform_coloring(setting::graph_edges_color);
                        edges->set_line_width(setting::graph_edges_line_width);
                        edges->set_impostor_type(LinesDrawable::CYLINDER);
                    }
                }
            } else
                edges->set_visible(!edges->is_visible());
        }
    } else if (e->key() == Qt::Key_V && e->modifiers() == Qt::NoModifier) {
        if (currentModel()) {
            auto vertices = currentModel()->renderer()->get_points_drawable("vertices");
            if (!vertices) {
                vertices = currentModel()->renderer()->add_points_drawable("vertices");
                if (dynamic_cast<SurfaceMesh *>(currentModel())) {
                    vertices->set_uniform_coloring(setting::surface_mesh_vertices_color);
                    vertices->set_impostor_type(PointsDrawable::SPHERE);
                    vertices->set_point_size(setting::surface_mesh_vertices_point_size);
                } else if (dynamic_cast<PointCloud *>(currentModel())) {
                    vertices->set_point_size(setting::point_cloud_point_size);
                    vertices->set_uniform_coloring(setting::point_cloud_points_color);
                } else if (dynamic_cast<Graph *>(currentModel())) {
                    vertices->set_uniform_coloring(setting::graph_vertices_color);
                    vertices->set_point_size(setting::graph_vertices_point_size);
                    vertices->set_impostor_type(PointsDrawable::SPHERE);
                }
            } else
                vertices->set_visible(!vertices->is_visible());
        }
    } else if (e->key() == Qt::Key_B && e->modifiers() == Qt::NoModifier) {
        SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(currentModel());
        if (mesh) {
            auto borders = mesh->renderer()->get_lines_drawable("borders");
            if (!borders) {
                borders = mesh->renderer()->add_lines_drawable("borders");
                borders->set_uniform_coloring(setting::surface_mesh_borders_color);
                borders->set_impostor_type(LinesDrawable::CYLINDER);
                borders->set_line_width(setting::surface_mesh_borders_line_width);
            } else
                borders->set_visible(!borders->is_visible());
        }
    } else if (e->key() == Qt::Key_L && e->modifiers() == Qt::NoModifier) { // locked vertices
        SurfaceMesh *mesh = dynamic_cast<SurfaceMesh *>(currentModel());
        if (mesh) {
            auto drawable = mesh->renderer()->get_points_drawable("locks");
            if (!drawable) {
                drawable = mesh->renderer()->add_points_drawable("locks");
                drawable->set_uniform_coloring(vec4(1, 1, 0, 1.0f));
                drawable->set_impostor_type(PointsDrawable::SPHERE);
                drawable->set_point_size(setting::surface_mesh_vertices_point_size + 5);
            } else
                drawable->set_visible(!drawable->is_visible());
        }
    } else if (e->key() == Qt::Key_M && e->modifiers() == Qt::NoModifier) {
        if (dynamic_cast<SurfaceMesh *>(currentModel())) {
            auto drawable = currentModel()->renderer()->get_triangles_drawable("faces");
            if (drawable) {
                drawable->set_smooth_shading(!drawable->smooth_shading());
            }
        }
    } else if (e->key() == Qt::Key_D && e->modifiers() == Qt::NoModifier) {
        if (currentModel()) {
            std::ostream &output = std::cout;

            output << "----------- " << file_system::simple_name(currentModel()->name()) << " -----------\n";
            if (dynamic_cast<SurfaceMesh *>(currentModel())) {
                auto model = dynamic_cast<SurfaceMesh *>(currentModel());
                output << "model is a surface mesh. #face: " << std::to_string(model->n_faces())
                       << ", #vertex: " + std::to_string(model->n_vertices())
                       << ", #edge: " + std::to_string(model->n_edges()) << std::endl;
            } else if (dynamic_cast<PointCloud *>(currentModel())) {
                auto model = dynamic_cast<PointCloud *>(currentModel());
                output << "model is a point cloud. #vertex: " + std::to_string(model->n_vertices()) << std::endl;
            } else if (dynamic_cast<Graph *>(currentModel())) {
                auto model = dynamic_cast<Graph *>(currentModel());
                output << "model is a graph. #vertex: " + std::to_string(model->n_vertices())
                       << ", #edge: " + std::to_string(model->n_edges()) << std::endl;
            }

            if (!currentModel()->renderer()->points_drawables().empty()) {
                output << "points drawables:\n";
                for (auto d : currentModel()->renderer()->points_drawables())
                    d->buffer_stats(output);
            }
            if (!currentModel()->renderer()->lines_drawables().empty()) {
                output << "lines drawables:\n";
                for (auto d : currentModel()->renderer()->lines_drawables())
                    d->buffer_stats(output);
            }
            if (!currentModel()->renderer()->triangles_drawables().empty()) {
                output << "triangles drawables:\n";
                for (auto d : currentModel()->renderer()->triangles_drawables())
                    d->buffer_stats(output);
            }

            currentModel()->property_stats(output);
        }
    } else if (e->key() == Qt::Key_R && e->modifiers() == Qt::NoModifier) {
        // Reload the shader(s) - useful for writing/debugging shader code.
        ShaderManager::reload();
    }

    QOpenGLWidget::keyPressEvent(e);
    update();
}


void ViewerQt::keyReleaseEvent(QKeyEvent *e) {
    QOpenGLWidget::keyReleaseEvent(e);
    update();
}


void ViewerQt::timerEvent(QTimerEvent *e) {
    QOpenGLWidget::timerEvent(e);
    update();
}


void ViewerQt::closeEvent(QCloseEvent *e) {
    cleanup();
    QOpenGLWidget::closeEvent(e);
}


std::string ViewerQt::usage() const {
    return std::string(
            " ------------------------------------------------------------------\n"
            " Easy3D viewer usage:                                              \n"
            " ------------------------------------------------------------------\n"
            "  F1:                  Help                                        \n"
            " ------------------------------------------------------------------\n"
            "  Ctrl + 'o':          Open file                                   \n"
            "  Ctrl + 's':          Save file                                   \n"
            "  Fn + Delete:         Delete current model                        \n"
            "  '<' or '>':          Switch between models                       \n"
            "  's':                 Snapshot                                    \n"
            " ------------------------------------------------------------------\n"
            "  'p':                 Toggle perspective/orthographic projection)	\n"
            "  Left:                Orbit-rotate the camera                     \n"
            "  Right:               Move up/down/left/right                     \n"
            "  Middle or Wheel:     Zoom in/out                                 \n"
            "  Ctrl + '+'/'-':      Zoom in/out                                 \n"
            "  Alt + Left:          Orbit-rotate the camera (screen based)      \n"
            "  Alt + Right:         Move up/down/left/right (screen based)      \n"
            "  Left/Right           Turn camera left/right                      \n"
            "  Ctrl + Left/Right:   Move camera left/right                      \n"
            "  Up/Down:             Move camera forward/backward                \n"
            "  Ctrl + Up/Down:      Move camera up/down                         \n"
            " ------------------------------------------------------------------\n"
            "  'f':                 Fit screen (all models)                     \n"
            "  'c':                 Fit screen (current model only)             \n"
            "  Shift + Left/Right:  Zoom to target/Zoom to fit screen           \n"
            " ------------------------------------------------------------------\n"
            "  '+'/'-':             Increase/Decrease point size (line width)   \n"
            "  'a':                 Toggle axes									\n"
            "  'b':                 Toggle borders								\n"
            "  'e':                 Toggle edges							    \n"
            "  'v':                 Toggle vertices                             \n"
            "  'm':                 Toggle smooth shading (for SurfaceMesh)     \n"
            "  'd':                 Print model info (drawables, properties)    \n"
            " ------------------------------------------------------------------\n"
    );
}


void ViewerQt::addModel(Model *model) {
    if (!model) {
        LOG(WARNING) << "model is NULL.";
        return;
    }
    for (auto m : models_) {
        if (model == m) {
            LOG(WARNING) << "model has alreay been added to the viewer.";
            return;
        }
    }

    if (model->empty()) {
        LOG(WARNING) << "model does not have vertices. Only complete model can be added to the viewer.";
        return;
    }

    auto renderer = new Renderer(model);
    model->set_renderer(renderer);

    int pre_idx = model_idx_;
    models_.push_back(model);
    model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current

    if (model_idx_ != pre_idx) {
        emit currentModelChanged();
        if (model_idx_ >= 0)
            LOG(INFO) << "current model: " << model_idx_ << ", " << models_[model_idx_]->name();
    }
}


void ViewerQt::deleteModel(Model *model) {
    if (!model) {
        LOG(WARNING) << "model is NULL.";
        return;
    }

    int pre_idx = model_idx_;
    auto pos = std::find(models_.begin(), models_.end(), model);
    if (pos != models_.end()) {
        const std::string name = model->name();
        models_.erase(pos);
        delete model;
        model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current

        std::cout << "model deleted: " << name << std::endl;
    } else
        LOG(WARNING) << "no such model: " << model->name();

    if (model_idx_ != pre_idx) {
        emit currentModelChanged();
        if (model_idx_ >= 0)
            LOG(INFO) << "current model: " << model_idx_ << ", " << models_[model_idx_]->name();
    }
}


void ViewerQt::fitScreen(const Model *model) {
    if (!model && models_.empty())
        return;

    Box3 box;
    if (model)
        box = model->bounding_box();
    else {
        for (auto m : models_)
            box.add_box(m->bounding_box());
    }
    camera_->setSceneBoundingBox(box.min(), box.max());
    camera_->showEntireScene();
    update();
}


vec3 ViewerQt::pointUnderPixel(const QPoint &p, bool &found) const {
    const_cast<ViewerQt *>(this)->makeCurrent();

    // Qt (same as GLFW) uses upper corner for its origin while GL uses the lower corner.
    int glx = p.x();
    int gly = height() - 1 - p.y();

    // NOTE: when dealing with OpenGL, all positions are relative to the viewer port.
    //       So we have to handle highdpi desplays.
    glx = static_cast<int>(glx * dpi_scaling());
    gly = static_cast<int>(gly * dpi_scaling());

    int samples = 0;
    func_->glGetIntegerv(GL_SAMPLES, &samples);
    easy3d_debug_log_gl_error;

    float depth = 1.0f;
    if (samples > 0) {
        opengl::read_depth_ms(depth, glx, gly);
        easy3d_debug_log_gl_error;
    } else {
        opengl::read_depth(depth, glx, gly);
        easy3d_debug_log_gl_error;
    }

    const_cast<ViewerQt *>(this)->doneCurrent();
    // here the glGetError() won't work because the OpenGL context is not current.
    // easy3d_debug_log_gl_error;

    found = depth < 1.0f;
    if (found) {
        // The input to unprojectedCoordinatesOf() is defined in the screen coordinate system
        vec3 point(p.x(), p.y(), depth);
        point = camera_->unprojectedCoordinatesOf(point);
        return point;
    }

    return vec3();
}


void ViewerQt::paintGL() {
    easy3d_debug_log_gl_error;

#if 1
    // QOpenGLWidget renders everything into a FBO. Internally it changes
    // QSurfaceFormat to always have samples = 0 and the OpenGL context is
    // not a multisample context. So we have to query the render-buffer
    // to know if it is using multisampling. At initializeGL() we were not
    // able to query the actual samples because the internal FBO has not
    // been created yet, so we do it here.
    static bool queried = false;
    if (!queried) {
#if 1
        func_->glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples_);
        easy3d_debug_log_frame_buffer_error;
#else   // the samples can also be retrieved using glGetIntegerv()
        func_->glGetIntegerv(GL_SAMPLES, &samples_); easy3d_debug_log_gl_error;
#endif
        // warn the user if the expected request was not satisfied
        int samples = QSurfaceFormat::defaultFormat().samples();
        int max_num = 0;
        func_->glGetIntegerv(GL_MAX_SAMPLES, &max_num);
        if (samples > 0 && samples_ != samples) {
            if (samples_ == 0)
                LOG(WARNING) << "MSAA is not available (" << samples << " samples requested)";
            else
                LOG(WARNING) << "MSAA is available with " << samples_ << " samples (" << samples
                             << " requested but max support is " << max_num << ")";
        } else
            LOG(INFO) << "Samples received: " << samples_ << " (" << samples << " requested, max support is "
                      << max_num << ")";

        queried = true;
    }
#endif

    preDraw();

    gpu_timer_->start();
    draw();
    gpu_timer_->stop();
    gpu_time_ = gpu_timer_->time();

    // Add visual hints: axis, camera, grid...
    postDraw();
}


void ViewerQt::drawCornerAxes() {
    ShaderProgram *program = ShaderManager::get_program("surface/surface_color");
    if (!program) {
        std::vector<ShaderProgram::Attribute> attributes;
        attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
        attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
        attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
        program = ShaderManager::create_program_from_files("surface/surface_color", attributes);
    }
    if (!program)
        return;

    if (!drawable_axes_) {
        const float base = 0.5f;   // the cylinder length, relative to the allowed region
        const float head = 0.2f;   // the cone length, relative to the allowed region
        std::vector<vec3> points, normals, colors;
        opengl::prepare_cylinder(0.03, 10, vec3(0, 0, 0), vec3(base, 0, 0), vec3(1, 0, 0), points, normals, colors);
        opengl::prepare_cylinder(0.03, 10, vec3(0, 0, 0), vec3(0, base, 0), vec3(0, 1, 0), points, normals, colors);
        opengl::prepare_cylinder(0.03, 10, vec3(0, 0, 0), vec3(0, 0, base), vec3(0, 0, 1), points, normals, colors);
        opengl::prepare_cone(0.06, 20, vec3(base, 0, 0), vec3(base + head, 0, 0), vec3(1, 0, 0), points, normals,
                             colors);
        opengl::prepare_cone(0.06, 20, vec3(0, base, 0), vec3(0, base + head, 0), vec3(0, 1, 0), points, normals,
                             colors);
        opengl::prepare_cone(0.06, 20, vec3(0, 0, base), vec3(0, 0, base + head), vec3(0, 0, 1), points, normals,
                             colors);
        opengl::prepare_sphere(vec3(0, 0, 0), 0.06, 20, 20, vec3(0, 1, 1), points, normals, colors);
        drawable_axes_ = new TrianglesDrawable("corner_axes");
        drawable_axes_->update_vertex_buffer(points);
        drawable_axes_->update_normal_buffer(normals);
        drawable_axes_->update_color_buffer(colors);
        drawable_axes_->set_property_coloring(State::VERTEX);
    }
    if (!drawable_axes_->is_visible())
        return;

    // The viewport and the scissor are changed to fit the lower left corner.
    int viewport[4], scissor[4];
    func_->glGetIntegerv(GL_VIEWPORT, viewport);
    func_->glGetIntegerv(GL_SCISSOR_BOX, scissor);

    static const int corner_frame_size = static_cast<int>(100 * dpi_scaling());
    func_->glViewport(0, 0, corner_frame_size, corner_frame_size);
    func_->glScissor(0, 0, corner_frame_size, corner_frame_size);

    // To make the axis appear over other objects: reserve a tiny bit of the
    // front depth range. NOTE: do remember to restore it later.
    func_->glDepthRangef(0, 0.01f);

    const mat4 &proj = transform::ortho(-1, 1, -1, 1, -1, 1);
    const mat4 &view = camera_->orientation().inverse().matrix();
    const mat4 &MVP = proj * view;

    // camera position is defined in world coordinate system.
    const vec3 &wCamPos = camera_->position();
    // it can also be computed as follows:
    //const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
    const mat4 &MV = camera_->modelViewMatrix();
    const vec4 &wLightPos = inverse(MV) * setting::light_position;

    program->bind();
    program->set_uniform("MVP", MVP);
    program->set_uniform("wLightPos", wLightPos);
    program->set_uniform("wCamPos", wCamPos);
    program->set_uniform("ssaoEnabled", false);
    program->set_uniform("per_vertex_color", true);
    program->set_uniform("two_sides_lighting", false);
    program->set_uniform("distinct_back_color", false);
    drawable_axes_->gl_draw(false);
    program->release();

    // restore
    func_->glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
    func_->glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    func_->glDepthRangef(0.0f, 1.0f);
}


void ViewerQt::preDraw() {
    // For normal drawing, i.e., drawing triggered by the paintEvent(),
    // the clearing is done before entering paintGL().
    // If you want to reuse the paintGL() method for offscreen rendering,
    // you have to clear both color and depth buffers beforehand.
    //func_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}


void ViewerQt::postDraw() {
    // draw the Easy3D logo and GPU time
    if (texter_ && texter_->num_fonts() >= 2) {
        const float font_size = 15.0f;
        const float offset = 20.0f * dpi_scaling();
        texter_->draw("Easy3D", offset, offset, font_size, 0);

        // the rendering time
        char buffer[48];
        sprintf(buffer, "Rendering (ms/frame): %4.1f", gpu_time_);
        texter_->draw(buffer, offset, 50.0f * dpi_scaling(), 16, 1);
    }

    if (show_pivot_point_) {
        ShaderProgram *program = ShaderManager::get_program("lines/lines_plain_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("lines/lines_plain_color", attributes);
        }
        if (!program)
            return;

        const float size = 10;
        LinesDrawable drawable("pivot_point");
        const vec3 &pivot = camera()->projectedCoordinatesOf(camera()->pivotPoint());
        std::vector<vec3> points = {
                vec3(pivot.x - size, pivot.y, 0.5f), vec3(pivot.x + size, pivot.y, 0.5f),
                vec3(pivot.x, pivot.y - size, 0.5f), vec3(pivot.x, pivot.y + size, 0.5f)
        };
        drawable.update_vertex_buffer(points);

        const mat4 &proj = transform::ortho(0.0f, static_cast<float>(width()), static_cast<float>(height()), 0.0f, 0.0f,
                                            -1.0f);
        glDisable(GL_DEPTH_TEST);   // always on top
        program->bind();
        program->set_uniform("MVP", proj);
        program->set_uniform("per_vertex_color", false);
        program->set_uniform("default_color", vec4(0.0f, 0.0f, 1.0f, 1.0f));
        drawable.gl_draw(false);
        program->release();
        glEnable(GL_DEPTH_TEST);   // restore
    }

    drawCornerAxes();
}


void ViewerQt::draw() {
    easy3d_debug_log_gl_error;

    for (const auto m : models_) {
        if (!m->renderer()->is_visible())
            continue;

        // temporarily change the depth range and depth comparison method to properly render edges.
        glDepthRange(0.001, 1.0);
        for (auto d : m->renderer()->triangles_drawables()) {
            if (d->is_visible())
                d->draw(camera(), false);
            easy3d_debug_log_gl_error;
        }

        glDepthRange(0.0, 1.0);
        glDepthFunc(GL_LEQUAL);
        for (auto d : m->renderer()->lines_drawables()) {
            if (d->is_visible())
                d->draw(camera(), false);
            easy3d_debug_log_gl_error;
        }
        glDepthFunc(GL_LESS);

        for (auto d : m->renderer()->points_drawables()) {
            if (d->is_visible())
                d->draw(camera(), false);
            easy3d_debug_log_gl_error;
        }
    }
}
