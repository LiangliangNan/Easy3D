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


#include "paint_canvas.h"
#include "main_window.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
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
#include <easy3d/renderer/walk_throuth.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>

#include <QKeyEvent>
#include <QPainter>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QApplication>
#include <QClipboard>
#include <QTime>
#include <QMessageBox>


using namespace easy3d;


PaintCanvas::PaintCanvas(MainWindow* window)
        : QOpenGLWidget(window)
        , window_(window)
        , func_(nullptr)
        , texter_(nullptr)
        , show_easy3d_logo_(true)
        , show_frame_rate_(true)
        , dpi_scaling_(1.0)
        , samples_(0)
        , camera_(nullptr)
        , background_color_(1.0f, 1.0f, 1.0f, 1.0f)
        , pressed_button_(Qt::NoButton)
        , modifiers_(Qt::NoModifier)
        , mouse_current_pos_(0, 0)
        , mouse_pressed_pos_(0, 0)
        , pressed_key_(-1)
        , show_pivot_point_(false)
        , drawable_axes_(nullptr)
        , show_labels_under_mouse_(false)
        , picked_face_index_(-1)
        , show_coordinates_under_mouse_(false)
        , model_idx_(-1)
        , ssao_(nullptr)
        , transparency_(nullptr)
        , shadow_(nullptr)
        , edl_(nullptr)
{
    // like Qt::StrongFocus plus the widget accepts focus by using the mouse wheel.
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    camera_ = new Camera;
    camera_->setType(Camera::PERSPECTIVE);
    camera_->setUpVector(vec3(0, 0, 1)); // Z pointing up
    camera_->setViewDirection(vec3(-1, 0, 0)); // X pointing out
    camera_->showEntireScene();

    easy3d::connect(&camera_->frame_modified, this, static_cast<void (PaintCanvas::*)(void)>(&PaintCanvas::update));

    walk_through_ = new WalkThrough(camera_);
}


PaintCanvas::~PaintCanvas() {
    // Make sure the context is current and then explicitly
    // destroy all underlying OpenGL resources.
    makeCurrent();

    cleanup();

    doneCurrent();
}


void PaintCanvas::cleanup() {
    for (auto m : models_) {
        delete m->renderer();
        delete m;
    }

    delete camera_;
    delete walk_through_;
    delete drawable_axes_;
    delete ssao_;
    delete shadow_;
    delete transparency_;
    delete edl_;
    delete texter_;

    ShaderManager::terminate();
    TextureManager::terminate();
}


void PaintCanvas::init() {
}


void PaintCanvas::initializeGL() {
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
    LOG(INFO) << "DPI scaling: " << dpi_scaling();

    // This won't work because QOpenGLWidget draws everything in framebuffer and
    // the framebuffer has not been created in the initializeGL() method. We
    // will query the actual samples in the paintGL() method.
    //int samples_received = 0;
    //func_->glgetintegerv(gl_samples, &samples_received);

    // create TextRenderer renderer and load default fonts
    texter_ = new TextRenderer(dpi_scaling());
    texter_->add_font(resource::directory() + "/fonts/en_Earth-Normal.ttf");
    texter_->add_font(resource::directory() + "/fonts/en_Roboto-Medium.ttf");

    timer_.start();

    // Calls user defined method.
    init();
}


void PaintCanvas::resizeGL(int w, int h) {
    QOpenGLWidget::resizeGL(w, h);

    // The viewport is set up by QOpenGLWidget before drawing.
    // So I don't need to set up.
    // func_->glViewport(0, 0, static_cast<int>(w * highdpi_), static_cast<int>(h * highdpi_));

    camera()->setScreenWidthAndHeight(w, h);
}


void PaintCanvas::setBackgroundColor(const vec4 &c) {
    background_color_ = c;

    makeCurrent();
    func_->glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
    doneCurrent();
}


void PaintCanvas::showEasy3DLogo(bool b) {
    show_easy3d_logo_ = b;
    update();
}


void PaintCanvas::showFrameRate(bool b) {
    show_frame_rate_ = b;
    update();
}


void PaintCanvas::showAxes(bool b) {
    if (drawable_axes_) {
        drawable_axes_->set_visible(b);
        update();
    }
}


void PaintCanvas::mousePressEvent(QMouseEvent *e) {
    pressed_button_ = e->button();
    modifiers_ = e->modifiers();
    mouse_current_pos_ = e->pos();
    mouse_pressed_pos_ = e->pos();

    if (tool_manager()->current_tool()) {
        tools::ToolButton bt = tools::NO_BUTTON;
        if (pressed_button_ == Qt::LeftButton)
            bt = tools::LEFT_BUTTON;
        else if (pressed_button_ == Qt::LeftButton)
            bt = tools::RIGHT_BUTTON;
        else if (pressed_button_ == Qt::MidButton)
            bt = tools::MIDDLE_BUTTON;
        if (bt != tools::NO_BUTTON) {
            makeCurrent();
            tool_manager()->press(bt, e->pos().x(), e->pos().y());
            doneCurrent();
        }
    }
    else {
        camera_->frame()->action_start();
        if (pressed_key_ == Qt::Key_Z || e->modifiers() == Qt::ShiftModifier) {
            if (e->button() == Qt::LeftButton) {
                bool found = false;
                const vec3 &p = pointUnderPixel(e->pos(), found);
                if (found) {
                    camera_->setPivotPoint(p);
                    // show, but hide the visual hint of pivot point after \p delay milliseconds.
                    show_pivot_point_ = true;
                    const int delay = 10000;
                    Timer::single_shot(delay, [&]() {
                        show_pivot_point_ = false;
                        update();
                    });
                    if (pressed_key_ == Qt::Key_Z && e->modifiers() == Qt::NoModifier)
                        camera()->interpolateToLookAt(p);
                } else {
                    camera_->setPivotPoint(camera_->sceneCenter());
                    show_pivot_point_ = false;
                }
            } else if (e->button() == Qt::RightButton) {
                camera_->setPivotPoint(camera_->sceneCenter());
                show_pivot_point_ = false;
                if (pressed_key_ == Qt::Key_Z && e->modifiers() == Qt::NoModifier)
                    camera()->interpolateToFitScene();
            }
        }

        if (e->modifiers() == Qt::AltModifier) {
            bool found = false;
            const vec3 p = pointUnderPixel(e->pos(), found);
            if (found && (walkThrough()->status() == easy3d::WalkThrough::WALKING_MODE) &&
                (!walkThrough()->interpolator()->is_interpolation_started()))
            {
                walkThrough()->walk_to(p);
            }
            else if (walkThrough()->status() == easy3d::WalkThrough::FREE_MODE) {
                LOG(WARNING) << "Alt + Left click is for the walking mode only. Use Ctrl + C instead";
            }
        }
    }

    QOpenGLWidget::mousePressEvent(e);
    update();
}


void PaintCanvas::mouseReleaseEvent(QMouseEvent *e) {
    if (tool_manager()->current_tool()) {
        tools::ToolButton bt = tools::NO_BUTTON;
        if (pressed_button_ == Qt::LeftButton)
            bt = tools::LEFT_BUTTON;
        else if (pressed_button_ == Qt::LeftButton)
            bt = tools::RIGHT_BUTTON;
        else if (pressed_button_ == Qt::MidButton)
            bt = tools::MIDDLE_BUTTON;
        if (bt != tools::NO_BUTTON) {
            makeCurrent();
            tool_manager()->release(bt, e->pos().x(), e->pos().y());
            doneCurrent();
        }

#if 1 // delete selected faces/points
        if (dynamic_cast<SurfaceMesh*>(currentModel())) {
            auto mesh = dynamic_cast<SurfaceMesh*>(currentModel());
            auto select = mesh->face_property<bool>("f:select");
            std::size_t count(0);
            for(auto f : mesh->faces()) {
                if (select[f]) {
                    mesh->delete_face(f);
                    ++count;
                }
            }
            mesh->collect_garbage();
            mesh->renderer()->update();
            LOG(INFO) << count << " faces deleted" << std::endl;
        } else if (dynamic_cast<PointCloud*>(currentModel())) {
            auto cloud = dynamic_cast<PointCloud*>(currentModel());
            auto select = cloud->vertex_property<bool>("v:select");
            std::size_t count(0);
            for(auto v : cloud->vertices()) {
                if (select[v]) {
                    cloud->delete_vertex(v);
                    ++count;
                }
            }
            cloud->collect_garbage();
            cloud->renderer()->update();
            LOG(INFO) << count << " points deleted" << std::endl;
        }
        window_->updateUi();
#endif
    }
    else if (pressed_button_ == Qt::LeftButton && e->modifiers() == Qt::ControlModifier) { // ZOOM_ON_REGION
        int xmin = std::min(mouse_pressed_pos_.x(), e->pos().x());
        int xmax = std::max(mouse_pressed_pos_.x(), e->pos().x());
        int ymin = std::min(mouse_pressed_pos_.y(), e->pos().y());
        int ymax = std::max(mouse_pressed_pos_.y(), e->pos().y());
        camera_->fitScreenRegion(xmin, ymin, xmax, ymax);
    }
    else
        camera_->frame()->action_end();

    pressed_button_ = Qt::NoButton;
    modifiers_ = Qt::NoModifier;
    mouse_current_pos_ = e->pos();

    QOpenGLWidget::mouseReleaseEvent(e);
    update();
}


void PaintCanvas::mouseMoveEvent(QMouseEvent *e) {
    int x = e->pos().x(), y = e->pos().y();
    if (x < 0 || x > width() || y < 0 || y > height() || walkThrough()->interpolator()->is_interpolation_started()) {
        e->ignore();
        return;
    }

    if (tool_manager()->current_tool()) {
        tools::ToolButton bt = tools::NO_BUTTON;
        if (pressed_button_ == Qt::LeftButton)
            bt = tools::LEFT_BUTTON;
        else if (pressed_button_ == Qt::LeftButton)
            bt = tools::RIGHT_BUTTON;
        else if (pressed_button_ == Qt::MidButton)
            bt = tools::MIDDLE_BUTTON;

        makeCurrent();
        tool_manager()->drag(bt, e->pos().x(), e->pos().y());
        doneCurrent();
    }
    else {
        // control modifier is reserved for zooming on region
        if (pressed_button_ != Qt::NoButton && modifiers_ != Qt::ControlModifier) {
            ManipulatedFrame *frame = camera()->frame();
            if (e->modifiers() == Qt::AltModifier) {
                if (setting::clipping_plane && setting::clipping_plane->is_enabled())
                    frame = setting::clipping_plane->manipulated_frame();
            }

            int x = e->pos().x();
            int y = e->pos().y();
            int dx = x - mouse_current_pos_.x();
            int dy = y - mouse_current_pos_.y();
            if (pressed_button_ == Qt::LeftButton)
                frame->action_rotate(x, y, dx, dy, camera_, pressed_key_ == Qt::Key_X);
            else if (pressed_button_ == Qt::RightButton)
                frame->action_translate(x, y, dx, dy, camera_, pressed_key_ == Qt::Key_X);
            else if (pressed_button_ == Qt::MidButton) {
                if (dy != 0 && frame == camera_->frame()) // zoom is intended for camera manipulation only
                    frame->action_zoom(dy > 0 ? 1 : -1, camera_);
            }
        }
    }

    mouse_current_pos_ = e->pos();
    QOpenGLWidget::mouseMoveEvent(e);

    if (show_labels_under_mouse_) {
        auto mesh = dynamic_cast<SurfaceMesh*>(currentModel());
        if (mesh) {
            makeCurrent();
            SurfaceMeshPicker picker(camera());
            picked_face_index_ = picker.pick_face(mesh, mouse_current_pos_.x(), mouse_current_pos_.y()).idx();
            doneCurrent();
        }
        else
            picked_face_index_ = -1;
    }

    if (show_coordinates_under_mouse_) {
        bool found = false;
        const easy3d::vec3 p = pointUnderPixel(e->pos(), found);
        QString coords= "XYZ = [-, -, -]";
        if (found)
            coords = QString("XYZ = [%1, %2, %3]").arg(p.x).arg(p.y).arg(p.z);
        window_->setPointUnderMouse(coords);
    }

    update();
}


void PaintCanvas::mouseDoubleClickEvent(QMouseEvent *e) {
    mouse_current_pos_ = e->pos();
    mouse_pressed_pos_ = e->pos();
    QOpenGLWidget::mouseDoubleClickEvent(e);
    update();
}


void PaintCanvas::wheelEvent(QWheelEvent *e) {
    if (tool_manager()->current_tool()) {
    }
    else {
        const int delta = e->delta();
        if (delta <= -1 || delta >= 1) {
            int dy = e->delta() > 0 ? 1 : -1;
            camera_->frame()->action_zoom(dy, camera_);
        }
    }

    QOpenGLWidget::wheelEvent(e);
    update();
}


Model *PaintCanvas::currentModel() {
    if (models_.empty())
        return nullptr;
    if (model_idx_ < models_.size())
        return models_[model_idx_];
    return nullptr;
}


void PaintCanvas::setCurrentModel(easy3d::Model *m) {
    for (int i=0; i<models_.size(); ++i) {
        if (models_[i] == m) {
            model_idx_ = i;
            return;
        }
    }
}


void PaintCanvas::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_F1) {
        std::cout << usage() << std::endl;
    }
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
    } else if (e->key() == Qt::Key_C && e->modifiers() == Qt::NoModifier) {
        if (currentModel())
            fitScreen(currentModel());
    } else if (e->key() == Qt::Key_F && e->modifiers() == Qt::NoModifier) {
        fitScreen();
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

    else if (e->key() == Qt::Key_BracketLeft && e->modifiers() == Qt::NoModifier) {
        for (auto m : models_) {
            for (auto d : m->renderer()->lines_drawables()) {
                float size = d->line_width() - 1.0f;
                if (size < 1)
                    size = 1;
                d->set_line_width(size);
                window_->updateRenderingPanel();
            }
        }
    } else if (e->key() == Qt::Key_BracketRight && e->modifiers() == Qt::NoModifier) {
        for (auto m : models_) {
            for (auto d : m->renderer()->lines_drawables()) {
                float size = d->line_width() + 1.0f;
                d->set_line_width(size);
                window_->updateRenderingPanel();
            }
        }
    } else if (e->key() == Qt::Key_Minus && e->modifiers() == Qt::NoModifier) {
        for (auto m : models_) {
            for (auto d : m->renderer()->points_drawables()) {
                float size = d->point_size() - 1.0f;
                if (size < 1)
                    size = 1;
                d->set_point_size(size);
                window_->updateRenderingPanel();
            }
        }
    } else if (e->key() == Qt::Key_Equal && e->modifiers() == Qt::NoModifier) {
        for (auto m : models_) {
            for (auto d : m->renderer()->points_drawables()) {
                float size = d->point_size() + 1.0f;
                d->set_point_size(size);
                window_->updateRenderingPanel();
            }
        }
    } else if (e->key() == Qt::Key_Comma && e->modifiers() == Qt::NoModifier) {
        int pre_idx = model_idx_;
        if (models_.empty())
            model_idx_ = -1;
        else
            model_idx_ = int((model_idx_ - 1 + models_.size()) % models_.size());
        if (model_idx_ != pre_idx) {
            window_->updateUi();
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
            window_->updateUi();
            if (model_idx_ >= 0)
				LOG(INFO) << "current model: " << model_idx_ << ", " << models_[model_idx_]->name();
        }
    } else if (e->key() == Qt::Key_Delete && e->modifiers() == Qt::NoModifier) {
        if (currentModel())
            deleteModel(currentModel());
        window_->updateUi();
    } else if (e->key() == Qt::Key_E && e->modifiers() == Qt::NoModifier) {
        if (currentModel()) {
            auto *edges = currentModel()->renderer()->get_lines_drawable("edges");
            if (edges) {
                edges->set_visible(!edges->is_visible());
                window_->updateRenderingPanel();
            }
        }
    } else if (e->key() == Qt::Key_V && e->modifiers() == Qt::NoModifier) {
        if (currentModel()) {
            auto vertices = currentModel()->renderer()->get_points_drawable("vertices");
            if (vertices) {
                vertices->set_visible(!vertices->is_visible());
                window_->updateRenderingPanel();
            }
        }
    }
    else if (e->key() == Qt::Key_B && e->modifiers() == Qt::NoModifier) {
        SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(currentModel());
        if (mesh) {
            auto borders = mesh->renderer()->get_lines_drawable("borders");
            if (borders) {
                borders->set_visible(!borders->is_visible());
                window_->updateRenderingPanel();
            }
        }
    }
    else if (e->key() == Qt::Key_L && e->modifiers() == Qt::NoModifier) { // locked vertices
        SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(currentModel());
        if (mesh) {
            auto drawable = mesh->renderer()->get_points_drawable("locks");
            if (drawable) {
                drawable->set_visible(!drawable->is_visible());
                window_->updateRenderingPanel();
            }
        }
    }
    else if (e->key() == Qt::Key_M && e->modifiers() == Qt::NoModifier) {
        if (dynamic_cast<SurfaceMesh *>(currentModel())) {
            auto drawable = currentModel()->renderer()->get_triangles_drawable("faces");
            if (drawable) {
                drawable->set_smooth_shading(!drawable->smooth_shading());
                window_->updateRenderingPanel();
            }
        }
    } else if (e->key() == Qt::Key_D && e->modifiers() == Qt::NoModifier) {
		if (currentModel()) {
		    std::ostream& output = std::cout;

            output << "----------- " << file_system::simple_name(currentModel()->name()) << " -----------\n";
			if (dynamic_cast<SurfaceMesh*>(currentModel())) {
				auto model = dynamic_cast<SurfaceMesh*>(currentModel());
                output << "model is a surface mesh. #face: " << std::to_string(model->n_faces())
						  << ", #vertex: " + std::to_string(model->n_vertices())
						  << ", #edge: " + std::to_string(model->n_edges()) << std::endl;
			}
			else if (dynamic_cast<PointCloud*>(currentModel())) {
				auto model = dynamic_cast<PointCloud*>(currentModel());
                output << "model is a point cloud. #vertex: " + std::to_string(model->n_vertices()) << std::endl;
			}
			else if (dynamic_cast<Graph*>(currentModel())) {
				auto model = dynamic_cast<Graph*>(currentModel());
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
    else
        pressed_key_ = e->key();

    QOpenGLWidget::keyPressEvent(e);
    update();
}


void PaintCanvas::keyReleaseEvent(QKeyEvent *e) {
    pressed_key_ = -1;

    QOpenGLWidget::keyReleaseEvent(e);
    update();
}


void PaintCanvas::timerEvent(QTimerEvent *e) {
    QOpenGLWidget::timerEvent(e);
    update();
}


void PaintCanvas::closeEvent(QCloseEvent *e) {
    cleanup();
    QOpenGLWidget::closeEvent(e);
}


std::string PaintCanvas::usage() const {
    return std::string(
            " ------------------------------------------------------------------\n"
            " Easy3D viewer usage:                                              \n"
            " ------------------------------------------------------------------\n"
            "  F1:                  Help                                        \n"
            " ------------------------------------------------------------------\n"
            "  Fn + Delete:         Delete current model                        \n"
            "  '<' or '>':          Switch between models                       \n"
            " ------------------------------------------------------------------\n"
            "  Left mouse:          Orbit-rotate the camera                     \n"
            "  Right mouse:         Pan-move the camera                         \n"
            "  'x' + Left mouse:    Orbit-rotate the camera (screen based)      \n"
            "  'x' + Right mouse:   Pan-move camera vertically or horizontally  \n"
            "  Middle or Wheel:     Zoom in/out                                 \n"
            "  Ctrl + '+'/'-':      Zoom in/out                                 \n"
            "  Left/Right           Turn camera left/right                      \n"
            "  Ctrl + Left/Right:   Move camera left/right                      \n"
            "  Up/Down:             Move camera forward/backward                \n"
            "  Ctrl + Up/Down:      Move camera up/down                         \n"
            " ------------------------------------------------------------------\n"
            "  'f':                 Fit screen (all models)                     \n"
            "  'c':                 Fit screen (current model only)             \n"
            "  'z' + Left mouse:    Zoom to target point on model               \n"
            "  'z' + Right mouse:   Zoom o fit screen                           \n"
            "  Shift + Left mouse:  Define a target point on model              \n"
            "  Shift + Right mouse: Undefine the target point (if any) on model \n"
            "  Ctrl + Left mouse:   Zoom to fit region                          \n"
            " ------------------------------------------------------------------\n"
            "  '-'/'=':             Decrease/Increase point size                \n"
            "  '{'/'}':             Decrease/Increase line width                \n"
            "  'b':                 Toggle borders								\n"
            "  'e':                 Toggle edges							    \n"
            "  'v':                 Toggle vertices                             \n"
            "  'm':                 Toggle smooth shading (for SurfaceMesh)     \n"
            "  'd':                 Print model info (drawables, properties)    \n"
            " ------------------------------------------------------------------\n"
    );
}


void PaintCanvas::addModel(Model *model) {
    if (!model) {
        LOG(WARNING) << "model is NULL.";
        return;
    }
    for (auto m : models_) {
        if (model == m) {
            LOG(WARNING) << "model '" << file_system::simple_name(model->name()) << "' has already been added to the viewer.";
            return;
        }
    }

    if (model->empty()) {
        LOG(WARNING) << "model does not have vertices. Only complete model can be added to the viewer.";
        return;
    }

    auto renderer = new Renderer(model);
    model->set_renderer(renderer);

    models_.push_back(model);
//    model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current
}


void PaintCanvas::deleteModel(Model *model) {
    if (!model) {
        LOG(WARNING) << "model is NULL.";
        return;
    }

    auto pos = std::find(models_.begin(), models_.end(), model);
    if (pos != models_.end()) {
        const std::string name = model->name();
        models_.erase(pos);
        delete model;
        model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current

        std::cout << "model deleted: " << name << std::endl;
    } else
        LOG(WARNING) << "no such model: " << model->name();
}


void PaintCanvas::fitScreen(const easy3d::Model *model) {
    if (!model && models_.empty())
        return;

    auto visual_box = [](const Model *m) -> Box3 {
        Box3 box = m->bounding_box();
        for (auto d : m->renderer()->points_drawables()) box.add_box(d->bounding_box());
        for (auto d : m->renderer()->lines_drawables()) box.add_box(d->bounding_box());
        for (auto d : m->renderer()->triangles_drawables()) box.add_box(d->bounding_box());
        return box;
    };

    Box3 box;
    if (model)
        box = visual_box(model);
    else {
        for (auto m : models_)
            box.add_box(visual_box(m));
    }

    if (box.is_valid()) {
        camera_->setSceneBoundingBox(box.min(), box.max());
        camera_->showEntireScene();
        update();
    }
}


vec3 PaintCanvas::pointUnderPixel(const QPoint &p, bool &found) const {
    const_cast<PaintCanvas *>(this)->makeCurrent();

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

    const_cast<PaintCanvas *>(this)->doneCurrent();

    found = depth < 1.0f;
    if (found) {
        // The input to unprojectedCoordinatesOf() is defined in the screen coordinate system
        return camera_->unprojectedCoordinatesOf(vec3(p.x(), p.y(), depth));
    }

    return vec3();
}


void PaintCanvas::paintGL() {
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

    draw();

    // Add visual hints: axis, camera, grid...
    postDraw();

    Q_EMIT drawFinished();
}


void PaintCanvas::drawCornerAxes() {
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
    func_->glDepthRangef(0, 0.01);

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
    program->set_uniform("MVP", MVP)
            ->set_uniform("lighting", true)
            ->set_uniform("two_sides_lighting", false)
            ->set_uniform("smooth_shading", true)
            ->set_uniform("wLightPos", wLightPos)
            ->set_uniform("wCamPos", wCamPos)
            ->set_uniform("ssaoEnabled", false)
            ->set_uniform("per_vertex_color", true)
            ->set_uniform("distinct_back_color", false)
            ->set_block_uniform("Material", "ambient", setting::material_ambient)
            ->set_block_uniform("Material", "specular", setting::material_specular)
            ->set_block_uniform("Material", "shininess", &setting::material_shininess)
            ->set_uniform("hightlight_id_min", -1)
            ->set_uniform("hightlight_id_max", -1)
            ->set_uniform("clippingPlaneEnabled", false); easy3d_debug_log_gl_error;

    drawable_axes_->gl_draw(false); easy3d_debug_log_gl_error;
    program->release();

    // restore
    func_->glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
    func_->glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    func_->glDepthRangef(0.0f, 1.0f);
}


void PaintCanvas::drawFaceAndVertexLabels(const QColor& face_color, const QColor& vertex_color) {
    SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(currentModel());
    if (!mesh || picked_face_index_ < 0 || picked_face_index_ >= mesh->n_faces())
        return;

    // using QPainter
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.beginNativePainting();

    // draw labels for the vertices of the picked face
    vec3 face_center(0, 0, 0);
    int valence = 0;
    for (auto v : mesh->vertices(SurfaceMesh::Face(picked_face_index_))) {
        const auto& p = mesh->position(v);
        face_center += p;
        const vec3 proj = camera()->projectedCoordinatesOf(p);
        painter.setPen(vertex_color);
        painter.drawText(proj.x, proj.y, QString::fromStdString("v" + std::to_string(v.idx())));
        ++valence;
    }

    // draw label for the picked face
    const vec3 proj = camera()->projectedCoordinatesOf(face_center/valence);
    painter.setPen(face_color);
    painter.drawText(proj.x, proj.y, QString::fromStdString("f" + std::to_string(picked_face_index_)));
    easy3d_debug_log_gl_error;

    // finish
    painter.endNativePainting();
    painter.end();
    // Liangliang: it seems QPainter disables depth test?
    func_->glEnable(GL_DEPTH_TEST);
}


void PaintCanvas::preDraw() {
    // For normal drawing, i.e., drawing triggered by the paintEvent(),
    // the clearing is done before entering paintGL().
    // If you want to reuse the paintGL() method for offscreen rendering,
    // you have to clear both color and depth buffers beforehand.
    //func_->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}


void PaintCanvas::postDraw() {
    easy3d_debug_log_gl_error;

    // draw the Easy3D logo and frame rate
    if (show_easy3d_logo_ && texter_ && texter_->num_fonts() >=2) {
        const float font_size = 15.0f;
        const float offset = 20.0f * dpi_scaling();
        texter_->draw("Easy3D", offset, offset, font_size, 0);
    }

    if (show_frame_rate_) {
        // FPS computation
        static unsigned int fpsCounter = 0;
        static double fps = 0.0;
        static const unsigned int maxCounter = 40;
        static QString fpsString("Rendering (Hz): --");
        if (++fpsCounter == maxCounter) {
            fps = 1000.0 * maxCounter / timer_.restart();
            fpsString = tr("Rendering (Hz): %1").arg(fps, 0, 'f', ((fps < 10.0) ? 1 : 0));
            fpsCounter = 0;
        }

#if 0   // draw frame rate text using Easy3D's built-in TextRenderer
        texter_->draw(fpsString.toStdString(), offset, 50.0f * dpi_scaling(), 16, 1);
#else   // draw frame rate text using Qt.
        QPainter painter; easy3d_debug_log_gl_error;
        painter.begin(this);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.beginNativePainting(); easy3d_debug_log_gl_error;
        painter.drawText(20, 50, fpsString);
        painter.endNativePainting();
        painter.end();  easy3d_debug_log_gl_error;
        func_->glEnable(GL_DEPTH_TEST); // it seems QPainter disables depth test?
#endif
    }

    // shown only when it is not animating
    if (walk_through_ && walk_through_->is_path_visible() && !walk_through_->interpolator()->is_interpolation_started())
        walk_through_->draw();
    easy3d_debug_log_gl_error;

    if (show_pivot_point_ || pressed_button_ != Qt::NoButton) {
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
        LinesDrawable drawable("pivot_point"); easy3d_debug_log_gl_error;
        const vec3 &pivot = camera()->projectedCoordinatesOf(camera()->pivotPoint());
        std::vector<vec3> points = {
                vec3(pivot.x - size, pivot.y, 0.5f), vec3(pivot.x + size, pivot.y, 0.5f),
                vec3(pivot.x, pivot.y - size, 0.5f), vec3(pivot.x, pivot.y + size, 0.5f)
        };
        drawable.update_vertex_buffer(points); easy3d_debug_log_gl_error;

        const mat4 &proj = transform::ortho(0.0f, width(), height(), 0.0f, 0.0f,-1.0f);
        glDisable(GL_DEPTH_TEST);   // always on top
        program->bind();
        program->set_uniform("MVP", proj);
        program->set_uniform("per_vertex_color", false);
        program->set_uniform("default_color", vec4(0.0f, 0.0f, 1.0f, 1.0f));
        drawable.gl_draw(false);
        program->release();
        glEnable(GL_DEPTH_TEST);   // restore
    }
    easy3d_debug_log_gl_error;

    if (show_labels_under_mouse_) {
        auto mesh = dynamic_cast<SurfaceMesh*>(currentModel());
        if (mesh && picked_face_index_ >= 0 && picked_face_index_ < mesh->n_faces()) {
            drawFaceAndVertexLabels(Qt::darkBlue, Qt::darkMagenta);
        }
    }

    // ------------- draw the picking region with transparency  ---------------

    if (pressed_button_ == Qt::LeftButton && modifiers_ == Qt::ControlModifier) {
        const Rect rect(mouse_pressed_pos_.x(), mouse_current_pos_.x(), mouse_pressed_pos_.y(), mouse_current_pos_.y());
        if (rect.width() > 0 || rect.height() > 0) {
            // draw the boundary of the rect
            opengl::draw_quad_wire(rect, vec4(0.0f, 0.0f, 1.0f, 1.0f), width(), height(), -1.0f);
            // draw the transparent face
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            opengl::draw_quad_filled(rect, vec4(0.0f, 0.0f, 1.0f, 0.2f), width(), height(), -0.9f);
            glDisable(GL_BLEND);
        }
    }

    // ------- draw the axes indicating the orientation of the model  ----------

    drawCornerAxes(); easy3d_debug_log_gl_error;

    // ------------------ draw elements from the tool --------------------------

    tool_manager()->draw_hint();    easy3d_debug_log_gl_error;

    // -------------------- draw the clipping plane ----------------------------

    //
    if (setting::clipping_plane)
        setting::clipping_plane->draw(camera());    easy3d_debug_log_gl_error;
}


void PaintCanvas::enableSsao(bool b) {
    if (b) {
        if (!ssao_)
            ssao_ = new AmbientOcclusion(camera_);
    }
    else {
        delete ssao_;
        ssao_ = nullptr;
    }

    for (const auto m : models_) {
        for (auto d : m->renderer()->lines_drawables()) {
            d->enable_ssao(b);
        }
        for (auto d : m->renderer()->points_drawables()) {
            d->enable_ssao(b);
        }
        for (auto d : m->renderer()->triangles_drawables()) {
            d->enable_ssao(b);
        }
    }
}


void PaintCanvas::enableShadow(bool b) {
    if (b) {
        if (!shadow_)
            shadow_ = new SoftShadow(camera_);

        if (transparency()) {
            delete transparency_;
            transparency_ = nullptr;
        }
    }
    else {
        delete shadow_;
        shadow_ = nullptr;
    }
}


void PaintCanvas::enableTransparency(bool b) {
    if (b) {
        if (!transparency_)
            transparency_ = new DualDepthPeeling(camera_);

        if (shadow()) {
            delete shadow_;
            shadow_ = nullptr;
        }
    }
    else {
        delete transparency_;
        transparency_ = nullptr;
    }
}


void PaintCanvas::enableEyeDomeLighting(bool b) {
    if (b) {
        if (!edl_)
            edl_ = new EyeDomeLighting(camera_);
    }
    else {
        delete edl_;
        edl_ = nullptr;
    }
}


void PaintCanvas::invertSelection() {
    LOG(WARNING) << "planned to be implemented in a future release";
}


void PaintCanvas::deleteSelectedPrimitives() {
    LOG(WARNING) << "planned to be implemented in a future release";
}


void PaintCanvas::setPerspective(bool b) {
    if (b)
        camera_->setType(Camera::PERSPECTIVE);
    else
        camera_->setType(Camera::ORTHOGRAPHIC);
    update();
}


void PaintCanvas::copyCamera() {
    const vec3 pos = camera()->position();
    const quat q = camera()->orientation();
    if (walkThrough()->status() == easy3d::WalkThrough::FREE_MODE) {
        walkThrough()->add_keyframe(Frame(camera()->position(), q));
    }
    else if (walkThrough()->status() == easy3d::WalkThrough::STOPPED) {
        const QString cam_str = QString("%1 %2 %3 %4 %5 %6 %7")
                .arg(pos[0])
                .arg(pos[1])
                .arg(pos[2])
                .arg(q[0])
                .arg(q[1])
                .arg(q[2])
                .arg(q[3]);
        qApp->clipboard()->setText(cam_str);
    }
    else if (walkThrough()->status() == easy3d::WalkThrough::WALKING_MODE)
        LOG(WARNING) << "Ctrl + C is for the free mode only. Use Alt + Left click instead";
}


void PaintCanvas::pasteCamera() {
    if (walkThrough()->interpolator()->is_interpolation_started() || walkThrough()->status() != easy3d::WalkThrough::STOPPED )
        return;

    // get the camera parameters from clipboard string
    const QString str = qApp->clipboard()->text();
    const QStringList list = str.split(" ", QString::SkipEmptyParts);
    if (list.size() != 7) {
        LOG(WARNING) << "camera not available in clipboard";
        return;
    }

    vec3 pos;
    for (int i = 0; i < 3; ++i) {
        bool ok;
        pos[i] = list[i].toFloat(&ok);
        if (!ok) {
            LOG(WARNING) << "camera not available in clipboard";
            return;
        }
    }

    quat orient;
    for (int i = 0; i < 4; ++i) {
        bool ok;
        orient[i] = list[i + 3].toDouble(&ok);
        if (!ok) {
            LOG(WARNING) << "camera not available in clipboard";
            return;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // change view directly
    // 	camera()->setPosition(pos);
    // 	camera()->setOrientation(orient);
    // I prefer to make animation
    Frame new_frame;
    new_frame.setPosition(pos);
    new_frame.setOrientation(orient);
    const float duration = 0.5f;
    camera()->interpolateTo(new_frame, duration);

    update();
}


void PaintCanvas::saveStateToFile(std::ofstream& output) const {
    // first line is just a comment
    output << "<Mapple>" << std::endl << std::endl;

    //-----------------------------------------------------

    // write foreground and background colors
    output << "<color>" << std::endl;
    output << "\t backGroundColor: " << backGroundColor() << std::endl;
    output << "</color>" << std::endl << std::endl;

    //-----------------------------------------------------

    output << "<display>" << std::endl;
    output << "\t cameraPathIsDrawn: " << walk_through_->is_path_visible() << std::endl;
    output << "</display>" << std::endl << std::endl;

    //-----------------------------------------------------

    output << "<windowState>" << std::endl;
    output << "\t state: " << window()->windowState() << std::endl;;
    if (window()->windowState() == Qt::WindowNoState) {
        output << "\t size: " << window()->width() << " " << window()->height() << std::endl;
        output << "\t position: " << window()->pos().x() << " " << window()->pos().y() << std::endl;
    }
    output << "</windowState>" << std::endl << std::endl;

    //-----------------------------------------------------

    output << "<camera>" << std::endl;

    switch (camera()->type()) {
        case Camera::PERSPECTIVE:	output << "\t type: " << "PERSPECTIVE" << std::endl;	break;
        case Camera::ORTHOGRAPHIC:	output << "\t type: " << "ORTHOGRAPHIC" << std::endl;	break;
    }
    output << "\t zClippingCoefficient: " << camera()->zClippingCoefficient() << std::endl;
    output << "\t zNearCoefficient: " << camera()->zNearCoefficient() << std::endl;
    output << "\t sceneRadius: " << camera()->sceneRadius() << std::endl;
    output << "\t fieldOfView: " << camera()->fieldOfView() << std::endl;
    output << "\t sceneCenter: " << camera()->sceneCenter() << std::endl;

    // ManipulatedCameraFrame
    output << "\t position: " << camera()->frame()->position() << std::endl;
    output << "\t orientation: " << camera()->frame()->orientation() << std::endl;
    output << "\t wheelSens: " << camera()->frame()->wheelSensitivity() << std::endl;
    output << "\t rotSens: " << camera()->frame()->rotationSensitivity() << std::endl;
    output << "\t zoomSens: " << camera()->frame()->zoomSensitivity() << std::endl;
    output << "\t transSens: " << camera()->frame()->translationSensitivity() << std::endl;
    output << "\t zoomsOnPivotPoint: " << camera()->frame()->zoomsOnPivotPoint() << std::endl;
    output << "\t pivotPoint: " << camera()->frame()->pivotPoint() << std::endl;

    output << "</camera>" << std::endl << std::endl;

    //-----------------------------------------------------

    output << "</Mapple>" << std::endl << std::endl;
}


void PaintCanvas::restoreStateFromFile(std::ifstream& input) {
    // first line is just a comment
    char line[500];
    input.getline(line, 500);	// just skip this line

    //-----------------------------------------------------

    // write foreground and background colors
    std::string dummy;
    input >> dummy;	// this skips the keyword
    vec4 bc;
    input >> dummy >> bc; setBackgroundColor(bc);
    input >> dummy;	// this skips the keyword

    //-----------------------------------------------------

    bool status = false;
    input >> dummy;	// this skips the keyword
    input >> dummy >> status;
    walk_through_->set_path_visible(status);
    input >> dummy;	// this skips the keyword

    //-----------------------------------------------------

    input >> dummy;	// this skips the keyword
    int state = -1;
    input >> dummy >> state;
    window()->setWindowState(Qt::WindowState(state));
    if (Qt::WindowState(state) == Qt::WindowNoState) {
        int w, h, x, y;
        input >> dummy >> w >> h;	window()->resize(w, h);
        input >> dummy >> x >> y;	window()->move(QPoint(x, y));
        camera()->setScreenWidthAndHeight(this->width(), this->height());
    }
    input >> dummy;	// this skips the keyword

    //-----------------------------------------------------

    input >> dummy;	// this skips the keyword

    // temporarily don't allow updating rendering when the camera parameters are changing.
    easy3d::disconnect(&camera_->frame_modified, this);

    std::string t;
    input >> dummy >> t;
    if (t == "PERSPECTIVE")
        camera()->setType(Camera::PERSPECTIVE);
    else if (t == "ORTHOGRAPHIC")
        camera()->setType(Camera::ORTHOGRAPHIC);

    float v(0);
    vec3 p;
    quat q;
    input >> dummy >> v;	camera()->setZClippingCoefficient(v);
    input >> dummy >> v;	camera()->setZNearCoefficient(v);
    input >> dummy >> v;	camera()->setSceneRadius(v);
    input >> dummy >> v;	camera()->setFieldOfView(v);
    input >> dummy >> p;	camera()->setSceneCenter(p);

    // ManipulatedCameraFrame
    input >> dummy >> p;	camera()->frame()->setPosition(p);
    input >> dummy >> q;	camera()->frame()->setOrientation(q);
    input >> dummy >> v;	camera()->frame()->setWheelSensitivity(v);
    input >> dummy >> v;	camera()->frame()->setRotationSensitivity(v);
    input >> dummy >> v;	camera()->frame()->setZoomSensitivity(v);
    input >> dummy >> v;	camera()->frame()->setTranslationSensitivity(v);
    input >> dummy >> status;	camera()->frame()->setZoomsOnPivotPoint(status);
    input >> dummy >> p;	camera()->frame()->setPivotPoint(p);
    input >> dummy;	// this skips the keyword

    //-----------------------------------------------------

    input >> dummy;	// the last keyword "</Mapple>"

    //-----------------------------------------------------

    // now camera parameters are all up-to-date, enable updating the rendering
    easy3d::connect(&camera_->frame_modified, this, static_cast<void (PaintCanvas::*)(void)>(&PaintCanvas::update));

    update();
}


void PaintCanvas::showFaceVertexLabelsUnderMouse(bool b) {
    show_labels_under_mouse_ = b;
    update();
}


void PaintCanvas::showCordinatesUnderMouse(bool b) {
    show_coordinates_under_mouse_ = b;

    QString coords("");
    if (show_coordinates_under_mouse_)
        coords = "XYZ = [-, -, -]";
    window_->setPointUnderMouse(coords);
}


void PaintCanvas::draw() {
    if (models_.empty())
        return;

    easy3d_debug_log_gl_error;
    // Optimization tips: rendering with multi-effects (e.g., shadowing, SSAO)
    // can benefit from sharing the same geometry pass.

    std::vector<TrianglesDrawable *> surfaces;
    for (auto m : models_) {
        if (!m->renderer()->is_visible())
            continue;
        for (auto d : m->renderer()->triangles_drawables())
            surfaces.push_back(d);
    }

    if (edl())
        edl()->begin();

    if (shadow()) {
        shadow()->draw(surfaces); easy3d_debug_log_gl_error;
        return;
    } else if (transparency()) {
        transparency()->draw(surfaces); easy3d_debug_log_gl_error;
        return;
    }
    else if (ssao()) {
        auto ssao_texture = ssao()->generate(models());
        for (const auto m : models_) {
            if (!m->renderer()->is_visible())
                continue;
            for (auto d : m->renderer()->lines_drawables())
                d->set_ssao_texture(ssao_texture);
            for (auto d : m->renderer()->points_drawables())
                d->set_ssao_texture(ssao_texture);
            for (auto d : m->renderer()->triangles_drawables())
                d->set_ssao_texture(ssao_texture);
        }
    }

    easy3d_debug_log_gl_error;

    for (const auto m : models_) {
        if (!m->renderer()->is_visible())
            continue;

        // Let's check if edges and surfaces are both shown. If true, we
        // make the depth coordinates of the surface smaller, so that displaying
        // the mesh and the surface together does not cause Z-fighting.
        std::size_t count = 0;
        for (auto d : m->renderer()->lines_drawables()) {
            if (d->is_visible()) {
                d->draw(camera(), false); easy3d_debug_log_gl_error;
                ++count;
            }
        }

        for (auto d : m->renderer()->points_drawables()) {
            if (d->is_visible())
                d->draw(camera(), false); easy3d_debug_log_gl_error;
        }

        if (count > 0) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(0.5f, -0.0001f);
        }
        for (auto d : m->renderer()->triangles_drawables()) {
            if (d->is_visible())
                d->draw(camera(), false); easy3d_debug_log_gl_error;
        }
        if (count > 0)
            glDisable(GL_POLYGON_OFFSET_FILL);
    }

    if (edl())
        edl()->end();
}
