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


#include "paint_canvas.h"
#include "main_window.h"
#include "walk_through.h"

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/graph.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/ambient_occlusion.h>
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/renderer/dual_depth_peeling.h>
#include <easy3d/renderer/eye_dome_lighting.h>
#include <easy3d/renderer/read_pixel.h>
#include <easy3d/renderer/opengl_util.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/text_renderer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/util/resource.h>
#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/gui/picker_model.h>
#include <easy3d/gui/picker.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/string.h>
#include <easy3d/util/line_stream.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/setting.h>

#include <QKeyEvent>
#include <QPainter>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QApplication>
#include <QClipboard>
#include <QTime>
#include <QStatusBar>

using namespace easy3d;

static struct Init {
    Init() { setting::background_color = vec4(1, 1, 1, 1); }
} init;

PaintCanvas::PaintCanvas(MainWindow* window)
        : QOpenGLWidget(window)
        , window_(window)
        , func_(nullptr)
        , texter_(nullptr)
        , show_easy3d_logo_(true)
        , show_frame_rate_(false)
        , dpi_scaling_(1.0)
        , samples_(0)
        , camera_(nullptr)
        , background_color_(setting::background_color)
        , pressed_button_(Qt::NoButton)
        , modifiers_(Qt::NoModifier)
        , mouse_current_pos_(0, 0)
        , mouse_pressed_pos_(0, 0)
        , pressed_key_(-1)
        , show_manip_sphere_(false)
        , drawable_axes_(nullptr)
        , drawable_manip_sphere_(nullptr)
        , model_picker_(nullptr)
        , allow_select_model_(false)
        , surface_mesh_picker_(nullptr)
        , show_primitive_id_under_mouse_(false)
        , show_primitive_property_under_mouse_(false)
        , picked_face_index_(-1)
        , point_cloud_picker_(nullptr)
        , picked_vertex_index_(-1)
        , show_coordinates_under_mouse_(false)
        , model_idx_(-1)
        , ssao_(nullptr)
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
    transparency_ = setting::effect_transparency_enabled ? (new DualDepthPeeling(camera_)) : nullptr;
    shadow_ = setting::effect_shadow_enabled ? (new SoftShadow(camera_)) : nullptr;
    edl_ = setting::effect_edl_enabled ? (new EyeDomeLighting(camera_)) : nullptr;
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
        delete m->manipulator();
        delete m;
    }

    delete camera_;
    delete walkThrough();
    delete drawable_axes_;
    delete drawable_manip_sphere_;
    delete ssao_;
    delete shadow_;
    delete transparency_;
    delete edl_;
    delete texter_;
    delete model_picker_;
    delete surface_mesh_picker_;
    delete point_cloud_picker_;

    ShaderManager::terminate();
    TextureManager::terminate();
}


void PaintCanvas::init() {
}


void PaintCanvas::initializeGL() {
    QOpenGLWidget::initializeGL();
    func_ = context()->functions();
    func_->initializeOpenGLFunctions();

    OpenglUtil::init();
#ifndef NDEBUG
    opengl::setup_gl_debug_callback();
#endif

    if (!func_->hasOpenGLFeature(QOpenGLFunctions::Multisample))
        throw std::runtime_error("Multisample not supported on this machine!!! Mapple may not run properly");
    if (!func_->hasOpenGLFeature(QOpenGLFunctions::Framebuffers))
        throw std::runtime_error(
                "Framebuffer Object is not supported on this machine!!! Mapple may not run properly");

    //// back face culling?
    //func_->glEnable(GL_CULL_FACE);
    //func_->glCullFace(GL_BACK); // culls only the back faces

    func_->glEnable(GL_DEPTH_TEST);
    func_->glDepthFunc(GL_LESS);

    func_->glClearDepthf(1.0f);
    func_->glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);

    int major_requested = QSurfaceFormat::defaultFormat().majorVersion();
    int minor_requested = QSurfaceFormat::defaultFormat().minorVersion();
    VLOG(1) << "OpenGL vendor: " << func_->glGetString(GL_VENDOR);
    VLOG(1) << "OpenGL renderer: " << func_->glGetString(GL_RENDERER);
    VLOG(1) << "OpenGL version requested: " << major_requested << "." << minor_requested;
    VLOG(1) << "OpenGL version received: " << func_->glGetString(GL_VERSION);
    VLOG(1) << "GLSL version received: " << func_->glGetString(GL_SHADING_LANGUAGE_VERSION);

    int major = 0;
    func_->glGetIntegerv(GL_MAJOR_VERSION, &major);
    int minor = 0;
    func_->glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major * 10 + minor < 32) {
        throw std::runtime_error("Mapple requires at least OpenGL 3.2");
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    dpi_scaling_ = static_cast<float>(devicePixelRatioF());
#else
    dpi_scaling_ = static_cast<float>(devicePixelRatio());
#endif
    VLOG(1) << "DPI scaling: " << dpi_scaling();

    // This won't work because QOpenGLWidget draws everything in framebuffer and
    // the framebuffer has not been created in the initializeGL() method. We
    // will query the actual samples in the paintGL() method.
    //int samples_received = 0;
    //func_->glgetintegerv(gl_samples, &samples_received);

    // create TextRenderer renderer and load default fonts
    texter_ = new TextRenderer(dpi_scaling());
    texter_->add_font(resource::directory() + "/fonts/en_Earth-Normal.ttf");
    texter_->add_font(resource::directory() + "/fonts/en_Roboto-Medium.ttf");

    // Calls user defined method.
    init();

    timer_.start();
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
    if (shadow_)
        shadow_->set_virtual_background_color(c);

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
        if (e->button() == Qt::LeftButton)
            bt = tools::LEFT_BUTTON;
        else if (e->button() == Qt::RightButton)
            bt = tools::RIGHT_BUTTON;
        else if (e->button() == Qt::MiddleButton)
            bt = tools::MIDDLE_BUTTON;
        if (bt != tools::NO_BUTTON) {
            makeCurrent();
            tool_manager()->press(bt, e->pos().x(), e->pos().y());
            doneCurrent();
        }
    }
    else {
        if (e->modifiers() == Qt::AltModifier) {
            bool found = false;
            const vec3 p = pointUnderPixel(e->pos(), found);
            if (found && (walkThrough()->status() == WalkThrough::WALKING_MODE) &&
                (!walkThrough()->interpolator()->is_interpolation_started()))
            {
                walkThrough()->walk_to(p);
                update();
            }
            else if (walkThrough()->status() == WalkThrough::ROTATE_AROUND_AXIS)
            {
                if (walkThrough()->interpolator()->number_of_keyframes() > 0) {
                    auto answer = dialog::message("Rotate camera around axis", "Overwrite the existing camera path?", dialog::Choice::ok_cancel, dialog::Type::warning);
                    if (answer == dialog::Response::cancel)
                        return;
                    else if (answer == dialog::Response::ok)
                        walkThrough()->interpolator()->delete_path();
                }

                Picker picker(camera_);
                const Line3 line = picker.picking_line(e->pos().x(), e->pos().y());
                walkThrough()->generate_camera_path(line);
                update();
            }
            else if (walkThrough()->status() == WalkThrough::FREE_MODE) {
                LOG(WARNING) << "press 'K' to add a keyframe in the free mode";
            }
        } else {
            if ((e->button() == Qt::LeftButton || e->button() == Qt::RightButton) && e->modifiers() != Qt::AltModifier && e->modifiers() != Qt::ControlModifier)
                show_manip_sphere_ = true;

            if (e->modifiers() == Qt::NoModifier && e->button() == Qt::LeftButton &&
                walkThrough()->status() == WalkThrough::STOPPED && allow_select_model_) {
                if (!model_picker_)
                    model_picker_ = new ModelPicker(camera());
                makeCurrent();
                auto model = model_picker_->pick(models(), e->pos().x(), e->pos().y());
                doneCurrent();
                if (model) {
                    setCurrentModel(model);
                    window_->updateUi();
                }
                // select the picked model (if picked) and deselect others
                for (auto m : models_)
                    m->renderer()->set_selected(m == model);
            }

            else {
                camera_->frame()->action_start();

                if (pressed_key_ == Qt::Key_Z || e->modifiers() == Qt::ShiftModifier) {
                    if (e->button() == Qt::LeftButton) {
                        bool found = false;
                        const vec3 &p = pointUnderPixel(e->pos(), found);
                        if (found) {
                            camera_->setPivotPoint(p);
                            if (pressed_key_ == Qt::Key_Z && e->modifiers() == Qt::NoModifier)
                                camera()->interpolateToLookAt(p);
                        } else
                            camera_->setPivotPoint(camera_->sceneCenter());
                    } else if (e->button() == Qt::RightButton) {
                        camera_->setPivotPoint(camera_->sceneCenter());
                        if (pressed_key_ == Qt::Key_Z && e->modifiers() == Qt::NoModifier)
                            camera()->interpolateToFitScene();
                    }
                }
            }
            update();
        }
    }

    QOpenGLWidget::mousePressEvent(e);
}


void PaintCanvas::mouseReleaseEvent(QMouseEvent *e) {
    if (tool_manager()->current_tool()) {
        tools::ToolButton bt = tools::NO_BUTTON;
        if (e->button() == Qt::LeftButton) {
            bt = tools::LEFT_BUTTON;
        }
        else if (e->button() == Qt::RightButton)
            bt = tools::RIGHT_BUTTON;
        else if (e->button() == Qt::MiddleButton)
            bt = tools::MIDDLE_BUTTON;
        if (bt != tools::NO_BUTTON) {
            makeCurrent();
            tool_manager()->release(bt, e->pos().x(), e->pos().y());
            doneCurrent();
            window_->updateRenderingPanel();
        }
        update();
    }
    else if (pressed_button_ == Qt::LeftButton && e->modifiers() == Qt::ControlModifier) { // ZOOM_ON_REGION
        int xmin = std::min(mouse_pressed_pos_.x(), e->pos().x());
        int xmax = std::max(mouse_pressed_pos_.x(), e->pos().x());
        int ymin = std::min(mouse_pressed_pos_.y(), e->pos().y());
        int ymax = std::max(mouse_pressed_pos_.y(), e->pos().y());
        camera_->fitScreenRegion(xmin, ymin, xmax, ymax);
    }
    else {
        if (e->button() == Qt::LeftButton || e->button() == Qt::RightButton)
            show_manip_sphere_ = false;
        camera_->frame()->action_end();
        update();
    }

    pressed_button_ = Qt::NoButton;
    modifiers_ = Qt::NoModifier;
    mouse_current_pos_ = e->pos();

    QOpenGLWidget::mouseReleaseEvent(e);
}


namespace internal {
    template<typename MODEL>
    std::string get_vertex_scalar_property(MODEL *model, typename MODEL::Vertex v, const std::string &name) {
        auto prop_float = model->template get_vertex_property<float>(name);
        if (prop_float) return std::to_string(prop_float[v]);
        auto prop_double = model->template get_vertex_property<double>(name);
        if (prop_double) return std::to_string(prop_double[v]);
        auto prop_int = model->template get_vertex_property<int>(name);
        if (prop_int) return std::to_string(prop_int[v]);
        auto prop_uint = model->template get_vertex_property<unsigned int>(name);
        if (prop_uint) return std::to_string(prop_uint[v]);
        auto prop_char = model->template get_vertex_property<char>(name);
        if (prop_char) return std::to_string(prop_char[v]);
        auto prop_uchar = model->template get_vertex_property<unsigned char>(name);
        if (prop_uchar) return std::to_string(prop_uchar[v]);
        auto prop_bool = model->template get_vertex_property<bool>(name);
        if (prop_bool) return std::to_string(prop_bool[v]);
        return "no scalar property is being visualized";
    }


    template<typename MODEL>
    std::string get_face_scalar_property(MODEL *model, typename MODEL::Face f, const std::string &name) {
        auto prop_float = model->template get_face_property<float>(name);
        if (prop_float) return std::to_string(prop_float[f]);
        auto prop_double = model->template get_face_property<double>(name);
        if (prop_double) return std::to_string(prop_double[f]);
        auto prop_int = model->template get_face_property<int>(name);
        if (prop_int) return std::to_string(prop_int[f]);
        auto prop_uint = model->template get_face_property<unsigned int>(name);
        if (prop_uint) return std::to_string(prop_uint[f]);
        auto prop_char = model->template get_face_property<char>(name);
        if (prop_char) return std::to_string(prop_char[f]);
        auto prop_uchar = model->template get_face_property<unsigned char>(name);
        if (prop_uchar) return std::to_string(prop_uchar[f]);
        auto prop_bool = model->template get_face_property<bool>(name);
        if (prop_bool) return std::to_string(prop_bool[f]);
        return "no scalar property is being visualized";
    }
}


void PaintCanvas::mouseMoveEvent(QMouseEvent *e) {
    int x = e->pos().x(), y = e->pos().y();
    if (x < 0 || x > width() || y < 0 || y > height() || walkThrough()->interpolator()->is_interpolation_started()) {
        e->ignore();
        return;
    }

    if (tool_manager()->current_tool()) {
        pressed_button_ = Qt::NoButton;
        tools::ToolButton bt = tools::NO_BUTTON;
        if (e->buttons() == Qt::LeftButton) {
            pressed_button_ = Qt::LeftButton;
            bt = tools::LEFT_BUTTON;
        }
        else if (e->buttons() == Qt::RightButton) {
            pressed_button_ = Qt::RightButton;
            bt = tools::RIGHT_BUTTON;
        }
        else if (e->buttons() == Qt::MiddleButton) {
            pressed_button_ = Qt::MiddleButton;
            bt = tools::MIDDLE_BUTTON;
        }

        makeCurrent();
        tool_manager()->move(bt, e->pos().x(), e->pos().y());
        doneCurrent();
        update();
    }
    else {
        // control modifier is reserved for zooming on region
        if (pressed_button_ != Qt::NoButton && modifiers_ != Qt::ControlModifier) {
            ManipulatedFrame *frame = camera()->frame();
            if (e->modifiers() == Qt::AltModifier) {
                if (ClippingPlane::instance()->is_enabled())
                    frame = ClippingPlane::instance()->manipulator()->frame();
                else if (currentModel() && currentModel()->renderer()->is_selected()) {
                    frame = currentModel()->manipulator()->frame();
                }
            }

            int dx = x - mouse_current_pos_.x();
            int dy = y - mouse_current_pos_.y();
            auto axis = ManipulatedFrame::NONE;
            if (pressed_key_ == Qt::Key_X) axis = ManipulatedFrame::HORIZONTAL;
            else if (pressed_key_ == Qt::Key_Y) axis = ManipulatedFrame::VERTICAL;
            else if (pressed_key_ == Qt::Key_O) axis = ManipulatedFrame::ORTHOGONAL;
            if (pressed_button_ == Qt::LeftButton)
                frame->action_rotate(x, y, dx, dy, camera_, axis);
            else if (pressed_button_ == Qt::RightButton)
                frame->action_translate(x, y, dx, dy, camera_, axis);
            else if (pressed_button_ == Qt::MiddleButton) {
                if (dy != 0 && frame == camera_->frame()) // zoom is intended for camera manipulation only
                    frame->action_zoom(dy > 0 ? 1 : -1, camera_);
            }
        }
    }

    if (show_primitive_id_under_mouse_ || show_primitive_property_under_mouse_) { // then need to pick primitives
        picked_face_index_ = -1;
        picked_vertex_index_ = -1;
        if (dynamic_cast<SurfaceMesh *>(currentModel())) {
            auto mesh = dynamic_cast<SurfaceMesh *>(currentModel());
            if (!surface_mesh_picker_)
                surface_mesh_picker_ = new SurfaceMeshPicker(camera());
            makeCurrent();
            auto picked_face = surface_mesh_picker_->pick_face(mesh, e->pos().x(), e->pos().y());
            doneCurrent();
            picked_face_index_ = picked_face.idx();

            // highlight the picked face in the model 
            // always treat the model as a general polygonal mesh.
            auto drawable = mesh->renderer()->get_triangles_drawable("faces");
            auto triangle_range = mesh->get_face_property<std::pair<int, int> >("f:triangle_range");
            if (triangle_range && picked_face.is_valid()) {
                const auto& range = triangle_range[picked_face];
                drawable->set_highlight_range(range);
                drawable->set_highlight(true);
            }
            else {
                drawable->set_highlight_range({-1, -1});
                drawable->set_highlight(false);
            }
            update();
        }
        else if (dynamic_cast<PointCloud *>(currentModel())) {
            auto cloud = dynamic_cast<PointCloud *>(currentModel());
            if (!point_cloud_picker_)
                point_cloud_picker_ = new PointCloudPicker(camera());
            makeCurrent();
            picked_vertex_index_ = point_cloud_picker_->pick_vertex(cloud, e->pos().x(), e->pos().y()).idx();
            doneCurrent();

            auto drawable = cloud->renderer()->get_points_drawable("vertices");
            if (picked_vertex_index_ >= 0) { // highlight the picked vertex
                drawable->set_highlight(true);
                drawable->set_highlight_range({picked_vertex_index_, picked_vertex_index_});
            }
            else {
                drawable->set_highlight(false);
                drawable->set_highlight_range({-1, -1});
            }
            update();
        }

        if (show_primitive_property_under_mouse_) {
            if (picked_vertex_index_ >= 0) { // picked point cloud vertex
                auto drawable = currentModel()->renderer()->get_points_drawable("vertices");
                if (drawable->coloring_method() == easy3d::State::SCALAR_FIELD) {
                    const std::string& name = drawable->property_name();
                    auto vertex = PointCloud::Vertex(picked_vertex_index_);
                    auto cloud = dynamic_cast<PointCloud *>(currentModel());
                    const std::string value_str = internal::get_vertex_scalar_property(cloud, vertex, name);
                    std::stringstream stream;
                    stream << "'" << name << "' on " << vertex << ": " << value_str;
                    window_->statusBar()->showMessage(QString::fromStdString(stream.str()), 2000);
                }
                else
                    window_->statusBar()->showMessage("no scalar property is being visualized", 2000);
            }
            else if (picked_face_index_ >= 0) { // picked surface mesh face
                auto drawable = currentModel()->renderer()->get_triangles_drawable("faces");
                if (drawable->coloring_method() == easy3d::State::SCALAR_FIELD && drawable->property_location() == easy3d::State::FACE) {
                    const std::string& name = drawable->property_name();
                    auto face = SurfaceMesh::Face(picked_face_index_);
                    auto mesh = dynamic_cast<SurfaceMesh *>(currentModel());
                    const std::string value_str = internal::get_face_scalar_property(mesh, face, name);
                    std::stringstream stream;
                    stream << "'" << name << "' on " << face << ": " << value_str;
                    window_->statusBar()->showMessage(QString::fromStdString(stream.str()), 2000);
                }
                else
                    window_->statusBar()->showMessage("no scalar property is being visualized", 2000);
            }
            else
                window_->statusBar()->showMessage("no primitive found under mouse", 2000);
        }
    }

    if (show_coordinates_under_mouse_) {
        bool found = false;
        const easy3d::vec3 p = pointUnderPixel(e->pos(), found);
        QString coords= "XYZ = [-, -, -]";
        if (found)
            coords = QString("XYZ = [%1, %2, %3]").arg(p.x).arg(p.y).arg(p.z);
        window_->statusBar()->showMessage(coords, 2000);
    }

    mouse_current_pos_ = e->pos();
    QOpenGLWidget::mouseMoveEvent(e);

//    if (pressed_button_ == Qt::LeftButton && modifiers_ == Qt::ControlModifier) // zoom on region
//        update();
}


void PaintCanvas::mouseDoubleClickEvent(QMouseEvent *e) {
    mouse_current_pos_ = e->pos();
    QOpenGLWidget::mouseDoubleClickEvent(e);
    update();
}


void PaintCanvas::wheelEvent(QWheelEvent *e) {
    if (walkThrough()->interpolator()->is_interpolation_started())
        return;

    if (tool_manager()->current_tool()) {
        update();
    }
    else {
        const int delta = e->angleDelta().y();
        if (delta <= -1 || delta >= 1) {
            int dy = e->angleDelta().y() > 0 ? 1 : -1;
            camera_->frame()->action_zoom(dy, camera_);
        }
    }

    QOpenGLWidget::wheelEvent(e);
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
    if (e->key() == Qt::Key_Left && e->modifiers() == Qt::KeypadModifier) {
        auto angle = static_cast<float>(1 * M_PI / 180.0); // turn left, 1 degrees each step
        camera_->frame()->action_turn(angle, camera_);
    } else if (e->key() == Qt::Key_Right && e->modifiers() == Qt::KeypadModifier) {
        auto angle = static_cast<float>(1 * M_PI / 180.0); // turn right, 1 degrees each step
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
    }

    else if (e->key() == Qt::Key_K && e->modifiers() == Qt::NoModifier) {
        if (walkThrough()->status() == WalkThrough::FREE_MODE) {
            const vec3 pos = camera()->position();
            const quat q = camera()->orientation();
            walkThrough()->add_keyframe(Frame(pos, q));
        }
        else if (walkThrough()->status() == WalkThrough::WALKING_MODE)
            LOG(WARNING) << "'K' is for the free mode only. Use Alt + Left click to add a keyframe in the walking mode";
    }
    else if (e->key() == Qt::Key_Space && e->modifiers() == Qt::NoModifier) {
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
        auto mesh = dynamic_cast<SurfaceMesh*>(currentModel());
        if (mesh) {
            auto borders = mesh->renderer()->get_lines_drawable("borders");
            if (borders) {
                borders->set_visible(!borders->is_visible());
                window_->updateRenderingPanel();
            }
        }
    }
    else if (e->key() == Qt::Key_L && e->modifiers() == Qt::NoModifier) { // locked vertices
        auto mesh = dynamic_cast<SurfaceMesh*>(currentModel());
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
    return  " ------------------------------------------------------------------\n"
            " Mapple usage:                                                     \n"
            " ------------------------------------------------------------------\n"
            "  Fn + Delete:         Delete current model                        \n"
            "  '<' or '>':          Switch between models                       \n"
            " ------------------------------------------------------------------\n"
            "  Left drag:           Rotate the camera                           \n"
            "  Right drag:          Move the camera                             \n"
            "  'x' + Left drag:     Rotate the camera around horizontal axis    \n"
            "  'x' + Right drag:    Move the camera along horizontal axis       \n"
            "  'y' + Left drag:     Rotate the camera around vertical axis      \n"
            "  'y' + Right drag:    Move the camera along vertical axis         \n"
            "  'o' + Left drag:     Rotate the camera around ortho-screen axis  \n"
            "  Middle or Wheel:     Zoom in/out                                 \n"
            "  Ctrl + '+'/'-':      Zoom in/out                                 \n"
            "  Left/Right           Turn camera left/right                      \n"
            "  Ctrl + Left/Right:   Move camera left/right                      \n"
            "  Up/Down:             Move camera forward/backward                \n"
            "  Ctrl + Up/Down:      Move camera up/down                         \n"
            "  Ctrl + 'c':          Copy current view status to clipboard       \n"
            "  Ctrl + 'v':          Restore view status from clipboard          \n"
            " ------------------------------------------------------------------\n"
            "  'f':                 Fit screen (all models)                     \n"
            "  'c':                 Fit screen (current model only)             \n"
            "  'z' + Left click:    Zoom to target point on model               \n"
            "  'z' + Right click:   Zoom o fit screen                           \n"
            "  Shift + Left click:  Define a target point on model              \n"
            "  Shift + Right click: Undefine the target point (if any) on model \n"
            "  Ctrl + Left drag:    Zoom to fit region                          \n"
            " ------------------------------------------------------------------\n"
            "  '-'/'=':             Decrease/Increase point size                \n"
            "  '{'/'}':             Decrease/Increase line width                \n"
            "  'b':                 Toggle borders								\n"
            "  'e':                 Toggle edges							    \n"
            "  'v':                 Toggle vertices                             \n"
            "  'm':                 Toggle smooth shading (for SurfaceMesh)     \n"
            "  'd':                 Print info about drawables and properties   \n"
            " ------------------------------------------------------------------\n";
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

    makeCurrent();
    model->set_renderer(new Renderer(model));
    model->set_manipulator(new Manipulator(model));
    model->manipulator()->frame()->modified.connect(this, static_cast<void (PaintCanvas::*)(void)>(&PaintCanvas::update));
    doneCurrent();

    models_.push_back(model);
    model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current
    adjustSceneRadius();
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
        makeCurrent();
        delete model->renderer();
        delete model->manipulator();
        delete model;
        doneCurrent();
        model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current
        LOG(INFO) << "model deleted: " << name;
    } else
        LOG(WARNING) << "no such model: " << model->name();
}


void PaintCanvas::fitScreen(const easy3d::Model *model) {
    if (!model && models_.empty())
        return;

    Box3 box_models;   // everything within box_models should be visible
    for (auto m : models_) {
        if (m->renderer()->is_visible())
            box_models.grow(m->bounding_box(false));
    }

    Box3 box_scene = box_models;   // everything within box_scene should be visible
    const std::size_t count = walkThrough()->interpolator()->number_of_keyframes();
    for (std::size_t i = 0; i < count; ++i)
        box_scene.grow(walkThrough()->interpolator()->keyframe(i).position());

    Box3 box;// this box will be fitted to the screen
    if (model && model->renderer()->is_visible())
        box = model->bounding_box(false);
    else
        box = box_models;

    if (!box.is_valid())
        return;

    // the proper scene radius (ensure everything within it will be visible) should be max of all distances from box's
    // center to all the vertices of box_scene
    float radius = box.radius();
    const std::vector<vec3> vts = {
            vec3(box_scene.min_coord(0), box_scene.min_coord(1), box_scene.min_coord(2)), // 0
            vec3(box_scene.max_coord(0), box_scene.min_coord(1), box_scene.min_coord(2)), // 1
            vec3(box_scene.max_coord(0), box_scene.max_coord(1), box_scene.min_coord(2)), // 2
            vec3(box_scene.min_coord(0), box_scene.max_coord(1), box_scene.min_coord(2)), // 3
            vec3(box_scene.min_coord(0), box_scene.min_coord(1), box_scene.max_coord(2)), // 4
            vec3(box_scene.max_coord(0), box_scene.min_coord(1), box_scene.max_coord(2)), // 5
            vec3(box_scene.max_coord(0), box_scene.max_coord(1), box_scene.max_coord(2)), // 6
            vec3(box_scene.min_coord(0), box_scene.max_coord(1), box_scene.max_coord(2)), // 7
    };
    for (const auto &p : vts)
        radius = std::max(radius, distance(box.center(), p));

    camera_->setSceneCenter(box.center());
    // make it a little bigger to ensure tiny objects (far away from other big objects) are also visible
    camera_->setSceneRadius(radius);
    camera_->fitBoundingBox(box.min_point(), box.max_point());
    update();
}


void PaintCanvas::fitScreen() {
    fitScreen(nullptr);
}


void PaintCanvas::adjustSceneRadius() {
    Box3 box;
    for (auto m : models_)
        box.grow(m->bounding_box(false));

    const std::size_t count = walkThrough()->interpolator()->number_of_keyframes();
    for (std::size_t i = 0; i < count; ++i)
        box.grow(walkThrough()->interpolator()->keyframe(i).position());

    // attention: the scene center is not changed.
    // Note: This is not accurate and just an estimate. If you see problem in rendering, use the max distance
    //       from the scene center to the eight vertices of the box.
    camera()->setSceneRadius(
            std::max(
                    distance(camera()->sceneCenter(), box.min_point()),
                    distance(camera()->sceneCenter(), box.max_point())
                    )
            );
}


vec3 PaintCanvas::pointUnderPixel(const QPoint &p, bool &found) const {
    // Qt (same as GLFW) uses upper corner for its origin while GL uses the lower corner.
    int glx = p.x();
    int gly = height() - 1 - p.y();

    // NOTE: when dealing with OpenGL, all positions are relative to the viewer port.
    //       So we have to handle highdpi displays.
    glx = static_cast<int>(static_cast<float>(glx) * dpi_scaling());
    gly = static_cast<int>(static_cast<float>(gly) * dpi_scaling());

    const_cast<PaintCanvas *>(this)->makeCurrent();

    int samples = 0;
    func_->glGetIntegerv(GL_SAMPLES, &samples);
    easy3d_debug_log_gl_error

    float depth = 1.0f;
    if (samples > 0) {
        opengl::read_depth_ms(depth, glx, gly);
        easy3d_debug_log_gl_error
    } else {
        opengl::read_depth(depth, glx, gly);
        easy3d_debug_log_gl_error
    }

    const_cast<PaintCanvas *>(this)->doneCurrent();

    found = depth < 1.0f;
    if (found) {
        // The input to unprojectedCoordinatesOf() is defined in the screen coordinate system
        return camera_->unprojectedCoordinatesOf(vec3(static_cast<float>(p.x()), static_cast<float>(p.y()), depth));
    }

    return {};
}


void PaintCanvas::paintGL() {
    easy3d_debug_log_gl_error

#if 1
    // QOpenGLWidget renders everything into an FBO. Internally it changes
    // QSurfaceFormat to always have samples = 0 and the OpenGL context is
    // not a multisample context. So we have to query the render-buffer
    // to know if it is using multisampling. At initializeGL() we were not
    // able to query the actual samples because the internal FBO has not
    // been created yet, so we do it here.
    static bool queried = false;
    if (!queried) {
#if 1
        func_->glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_SAMPLES, &samples_);
        easy3d_debug_log_frame_buffer_error
#else   // the samples can also be retrieved using glGetIntegerv()
        func_->glGetIntegerv(GL_SAMPLES, &samples_); easy3d_debug_log_gl_error
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
            VLOG(1) << "Samples received: " << samples_ << " (" << samples << " requested, max support is "
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
    ShaderProgram *program = ShaderManager::get_program("surface/surface");
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

    if (!drawable_axes_) {
        const float base = 0.5f;   // the cylinder length, relative to the allowed region
        const float head = 0.2f;   // the cone length, relative to the allowed region
        std::vector<vec3> points, normals, colors;
        shape::create_cylinder(0.03, 10, vec3(0, 0, 0), vec3(base, 0, 0), vec3(1, 0, 0), points, normals, colors);
        shape::create_cylinder(0.03, 10, vec3(0, 0, 0), vec3(0, base, 0), vec3(0, 1, 0), points, normals, colors);
        shape::create_cylinder(0.03, 10, vec3(0, 0, 0), vec3(0, 0, base), vec3(0, 0, 1), points, normals, colors);
        shape::create_cone(0.06, 20, vec3(base, 0, 0), vec3(base + head, 0, 0), vec3(1, 0, 0), points, normals,
                             colors);
        shape::create_cone(0.06, 20, vec3(0, base, 0), vec3(0, base + head, 0), vec3(0, 1, 0), points, normals,
                             colors);
        shape::create_cone(0.06, 20, vec3(0, 0, base), vec3(0, 0, base + head), vec3(0, 0, 1), points, normals,
                             colors);
        shape::create_sphere(vec3(0, 0, 0), 0.06, 20, 20, vec3(0, 1, 1), points, normals, colors);
        drawable_axes_ = new TrianglesDrawable("corner_axes");
        drawable_axes_->update_vertex_buffer(points);
        drawable_axes_->update_normal_buffer(normals);
        drawable_axes_->update_color_buffer(colors);
        drawable_axes_->set_property_coloring(State::VERTEX);
    }
    if (!drawable_axes_->is_visible())
        return;

    // The viewport is changed to fit the lower left corner.
    int viewport[4];
    func_->glGetIntegerv(GL_VIEWPORT, viewport);

    static const int corner_frame_size = static_cast<int>(100 * dpi_scaling());
    func_->glViewport(0, 0, corner_frame_size, corner_frame_size);

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
    program->set_uniform("MVP", MVP)
            ->set_uniform("MANIP", mat4::identity())
            ->set_uniform("NORMAL", mat3::identity())   // needs be padded when using uniform blocks
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
            ->set_uniform("highlight", false)
            ->set_uniform("clippingPlaneEnabled", false)
            ->set_uniform("selected", false)
            ->set_uniform("highlight_color", setting::highlight_color)
            ->set_uniform("use_texture", false);  easy3d_debug_log_gl_error

    drawable_axes_->gl_draw(); easy3d_debug_log_gl_error
    program->release();

    // restore the viewport
    func_->glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    func_->glDepthRangef(0.0f, 1.0f);
}


void PaintCanvas::drawPickedFaceAndItsVerticesIDs(const QColor& face_color, const QColor& vertex_color) {
    auto mesh = dynamic_cast<SurfaceMesh*>(currentModel());
    if (!mesh || picked_face_index_ < 0 || picked_face_index_ >= static_cast<int>(mesh->n_faces()))
        return;

    // using QPainter
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.beginNativePainting();

    mat4 manip = mesh->manipulator()->matrix();

    // draw labels for the vertices of the picked face
    vec3 face_center(0, 0, 0);
    int valence = 0;
    for (auto v : mesh->vertices(SurfaceMesh::Face(picked_face_index_))) {
        const auto& p = mesh->position(v);
        face_center += p;
        const vec3 proj = camera()->projectedCoordinatesOf(manip * p);
        painter.setPen(vertex_color);
        painter.drawText(static_cast<int>(proj.x), static_cast<int>(proj.y), QString::fromStdString("v" + std::to_string(v.idx())));
        ++valence;
    }

    // draw label for the picked face
    const vec3 proj = camera()->projectedCoordinatesOf(manip * (face_center/valence));
    painter.setPen(face_color);
    painter.drawText(static_cast<int>(proj.x), static_cast<int>(proj.y), QString::fromStdString("f" + std::to_string(picked_face_index_)));
    easy3d_debug_log_gl_error

    // finish
    painter.endNativePainting();
    painter.end();
    // Liangliang: it seems QPainter disables depth test?
    func_->glEnable(GL_DEPTH_TEST);
}


void PaintCanvas::drawPickedVertexID(const QColor &vertex_color) {
    auto cloud = dynamic_cast<PointCloud*>(currentModel());
    if (!cloud || picked_vertex_index_ < 0 || picked_vertex_index_ >= static_cast<int>(cloud->n_vertices()))
        return;

    // using QPainter
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.beginNativePainting();

    mat4 manip = cloud->manipulator()->matrix();

    // draw the ID of the picked vertex
    auto vertex = PointCloud::Vertex(picked_vertex_index_);
    vec3 pos = cloud->position(vertex);
    pos = camera()->projectedCoordinatesOf(manip * pos);
    painter.setPen(vertex_color);
    painter.drawText(static_cast<int>(pos.x), static_cast<int>(pos.y), QString::fromStdString("v" + std::to_string(picked_vertex_index_)));
    easy3d_debug_log_gl_error

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
    easy3d_debug_log_gl_error

    // draw the Easy3D logo and frame rate
    if (show_easy3d_logo_ && texter_ && texter_->num_fonts() >=2) {
        const float font_size = 15.0f;
        const float offset = 20.0f * dpi_scaling();
        texter_->draw("Easy3D", offset, offset, font_size, 0);
    }

    if (show_frame_rate_) {
        // FPS computation
        static unsigned int fps_count = 0;
        static double fps = 0.0;
        static const unsigned int max_count = 40;
        static QString fps_string("fps: ??");
        if (++fps_count == max_count) {
            fps = 1000.0 * max_count / static_cast<double>(timer_.restart());
            fps_string = tr("fps: %1").arg(fps, 0, 'f', ((fps < 10.0) ? 1 : 0));
            fps_count = 0;
        }

#if 0   // draw frame rate text using Easy3D's built-in TextRenderer
        if (texter_ && texter_->num_fonts() >=2)
            texter_->draw(fps_string.toStdString(), 20.0f * dpi_scaling(), 50.0f * dpi_scaling(), 16, 1);
#else   // draw frame rate text using Qt.
        QPainter painter; easy3d_debug_log_gl_error
        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setPen(Qt::black);
        painter.beginNativePainting(); easy3d_debug_log_gl_error
        painter.drawText(20, 50, fps_string);
        painter.endNativePainting();
        painter.end();  easy3d_debug_log_gl_error
        func_->glEnable(GL_DEPTH_TEST); // it seems QPainter disables depth test?
#endif
    }

    // shown only when it is not animating
    if (walkThrough() && !walkThrough()->interpolator()->is_interpolation_started())
        walkThrough()->draw();
    easy3d_debug_log_gl_error

    if (show_primitive_id_under_mouse_) {
        drawPickedFaceAndItsVerticesIDs(Qt::darkBlue, Qt::darkMagenta);
        drawPickedVertexID(Qt::darkMagenta);
    }

    // ------------- draw the picking region with transparency  ---------------

    if (pressed_button_ == Qt::LeftButton && modifiers_ == Qt::ControlModifier) {
        const Rect rect(static_cast<float>(mouse_pressed_pos_.x()), static_cast<float>(mouse_current_pos_.x()), static_cast<float>(mouse_pressed_pos_.y()), static_cast<float>(mouse_current_pos_.y()));
        if (rect.width() > 0 || rect.height() > 0) {
            // draw the boundary of the rect
            shape::draw_quad_wire(rect, vec4(0.0f, 0.0f, 1.0f, 1.0f), width(), height(), -1.0f);
            // draw the transparent face
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            shape::draw_quad_filled(rect, vec4(0.0f, 0.0f, 1.0f, 0.2f), width(), height(), -0.9f);
            glDisable(GL_BLEND);
        }
    }

    // ------- draw the axes indicating the orientation of the model  ----------

    drawCornerAxes(); easy3d_debug_log_gl_error

    // ------------------ draw elements from the tool --------------------------

    tool_manager()->draw_hint();    easy3d_debug_log_gl_error

    // -------------------- draw the clipping plane ----------------------------

    ClippingPlane::instance()->draw(camera());    easy3d_debug_log_gl_error

    // -------------------- draw a sphere outline

    Model* m = currentModel();
    if (m && m->renderer()->is_visible() && m->renderer()->is_selected()) {
        m->manipulator()->draw_frame(camera_);
    }
    else if (show_manip_sphere_) {
        if (!drawable_manip_sphere_)
            drawable_manip_sphere_ = new LinesDrawable;

        const int radius = 150; // pixels
        float ratio = camera_->pixelGLRatio(camera_->pivotPoint());
        auto manip = mat4::translation(camera_->pivotPoint()) * mat4::scale(radius * ratio) ;
        shape::draw_sphere_big_circles(drawable_manip_sphere_, camera_->modelViewProjectionMatrix(), manip);
    }
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
    makeCurrent();
    if (dynamic_cast<SurfaceMesh*>(currentModel())) {
        auto mesh = dynamic_cast<SurfaceMesh *>(currentModel());
        auto d = mesh->renderer()->get_triangles_drawable("faces");
        if (d->coloring_method() != easy3d::State::SCALAR_FIELD || d->property_name() != "f:select") {
            auto select = mesh->get_face_property<bool>("f:select");
            if (!select)
                select = mesh->add_face_property<bool>("f:select", false);
            for (auto f : mesh->faces())
                select[f] = !select[f];
            d->set_coloring(State::SCALAR_FIELD, State::FACE, "f:select");
            buffer::update(mesh, d);
        }
        else {
            auto select = mesh->face_property<bool>("f:select");
            for (auto f : mesh->faces())
                select[f] = !select[f];

            auto triangle_range = mesh->face_property<std::pair<int, int> >("f:triangle_range");

            // update the drawable's texcoord buffer
            std::vector<vec2> texcoords(d->num_vertices());
            for (auto f : mesh->faces()) {
                int start = triangle_range[f].first;
                int end = triangle_range[f].second;
                for (int idx = start; idx <= end; ++idx)
                    texcoords[idx * 3] = texcoords[idx * 3 + 1] = texcoords[idx * 3 + 2] = vec2(select[f],0.5f);
            }
            d->update_texcoord_buffer(texcoords);
            d->set_coloring(State::SCALAR_FIELD, State::FACE, "f:select");
        }
    } else if (dynamic_cast<PointCloud*>(currentModel())) {
        auto cloud = dynamic_cast<PointCloud*>(currentModel());
        auto d = cloud->renderer()->get_points_drawable("vertices");
        if (d->coloring_method() != easy3d::State::SCALAR_FIELD || d->property_name() != "v:select") {
            auto select = cloud->get_vertex_property<bool>("v:select");
            if (!select)
                select = cloud->add_vertex_property<bool>("v:select", false);
            for(auto v : cloud->vertices())
                select[v] = !select[v];
            d->set_coloring(State::SCALAR_FIELD, State::VERTEX, "v:select");
            buffer::update(cloud, d);
        }
        else {
            auto select = cloud->vertex_property<bool>("v:select");
            for (auto v : cloud->vertices())
                select[v] = !select[v];

            // update the drawable's texcoord buffer
            std::vector<vec2> texcoords(d->num_vertices());
            for (auto v : cloud->vertices())
                texcoords[v.idx()] = vec2(select[v], 0.5f);
            d->update_texcoord_buffer(texcoords);
            d->set_coloring(State::SCALAR_FIELD, State::VERTEX, "v:select");
        }
    }
    doneCurrent();

    window_->updateUi();
    update();
}


void PaintCanvas::deleteSelectedPrimitives() {
    if (dynamic_cast<SurfaceMesh*>(currentModel())) {
        auto mesh = dynamic_cast<SurfaceMesh*>(currentModel());
        auto select_faces = mesh->face_property<bool>("f:select");
        std::size_t count(0);
        for(auto f : mesh->faces()) {
            if (select_faces[f]) {
                mesh->delete_face(f);
                ++count;
            }
        }
        mesh->collect_garbage();
        mesh->manipulator()->reset();
        mesh->renderer()->update();
        LOG(INFO) << count << " faces deleted";
    } else if (dynamic_cast<PointCloud*>(currentModel())) {
        auto cloud = dynamic_cast<PointCloud*>(currentModel());
        auto select_vertices = cloud->vertex_property<bool>("v:select");
        std::size_t count(0);
        for(auto v : cloud->vertices()) {
            if (select_vertices[v]) {
                cloud->delete_vertex(v);
                ++count;
            }
        }
        cloud->collect_garbage();
        cloud->manipulator()->reset();
        cloud->renderer()->update();
        LOG(INFO) << count << " points deleted";
    }
    window_->updateUi();
    update();
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


void PaintCanvas::pasteCamera() {
    if (walkThrough()->interpolator()->is_interpolation_started() || walkThrough()->status() != WalkThrough::STOPPED )
        return;

    // get the camera parameters from clipboard string
    const QString str = qApp->clipboard()->text();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    const QStringList list = str.split(" ", Qt::SkipEmptyParts);
#else
    const QStringList list = str.split(" ", QString::SkipEmptyParts);
#endif
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
        orient[i] = static_cast<float>(list[i + 3].toDouble(&ok));
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


void PaintCanvas::saveState(std::ostream& output) const {
    if (output.fail())
        return;

    // first line is just a comment
    output << "<Mapple Viewer State> - Saved on " << string::date_time() << std::endl << std::endl;

    //-----------------------------------------------------

    // write foreground and background colors
    output << "<color>" << std::endl;
    output << "\t backGroundColor: " << backGroundColor() << std::endl;
    output << "</color>" << std::endl << std::endl;

    //-----------------------------------------------------

    output << "<windowState>" << std::endl;
    output << "\t state: " << window()->windowState() << std::endl;
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

    output << "</Mapple Viewer State>";
}


void PaintCanvas::restoreState(std::istream& input) {
    if (input.fail())
        return;

    // temporarily don't allow updating rendering when the camera parameters are changing.
    easy3d::disconnect_all(&camera_->frame_modified);

    std::string dummy;
    io::LineInputStream in(input);
    while (!input.eof()) {
        in.get_line();
        if (in.current_line()[0] != '<') {
            std::string keyword;
            in >> keyword;
            if (!in.fail()) {
                if (keyword.find("backGroundColor") != std::string::npos ||
                    keyword.find("background") != std::string::npos) {
                    vec3 bc;
                    in >> bc;
                    if (bc.x > 1.0f || bc.y > 1.0f || bc.z > 1.0f)
                        bc /= 255.0f;
                    setBackgroundColor(vec4(bc, 1.0f));
                } else if (keyword.find("state") != std::string::npos) {
                    int state = 0; // this is Qt::WindowNoState
                    in >> state;
                    window()->setWindowState(Qt::WindowState(state));
                    if (Qt::WindowState(state) == Qt::WindowNoState) {
                        int w, h, x, y;
                        in.get_line();
                        in >> dummy >> w >> h;
                        window()->resize(w, h);
                        in.get_line();
                        in >> dummy >> x >> y;
                        window()->move(QPoint(x, y));
                        camera()->setScreenWidthAndHeight(this->width(), this->height());
                    }
                } else if (keyword.find("type") != std::string::npos) {
                    std::string t;
                    in >> t;
                    if (t == "PERSPECTIVE")
                        camera()->setType(Camera::PERSPECTIVE);
                    else if (t == "ORTHOGRAPHIC")
                        camera()->setType(Camera::ORTHOGRAPHIC);
                } else if (keyword.find("zClippingCoefficient") != std::string::npos) {
                    float v(0);
                    in >> v;
                    camera()->setZClippingCoefficient(v);
                } else if (keyword.find("zNearCoefficient") != std::string::npos) {
                    float v(0);
                    in >> v;
                    camera()->setZNearCoefficient(v);
                } else if (keyword.find("sceneRadius") != std::string::npos) {
                    float v(0);
                    in >> v;
                    camera()->setSceneRadius(v);
                } else if (keyword.find("fieldOfView") != std::string::npos) {
                    float v(0);
                    in >> v;
                    camera()->setFieldOfView(v);
                } else if (keyword.find("sceneCenter") != std::string::npos) {
                    vec3 p;
                    in >> p;
                    camera()->setSceneCenter(p);
                } else if (keyword.find("position") != std::string::npos) {
                    vec3 p;
                    in >> p;
                    camera()->setPosition(p);
                } else if (keyword.find("orientation") != std::string::npos) {
                    quat q;
                    in >> q;
                    camera()->setOrientation(q);
                } else if (keyword.find("wheelSens") != std::string::npos) {
                    float v(0);
                    in >> v;
                    camera()->frame()->setWheelSensitivity(v);
                } else if (keyword.find("rotSens") != std::string::npos) {
                    float v(0);
                    in >> v;
                    camera()->frame()->setRotationSensitivity(v);
                } else if (keyword.find("zoomSens") != std::string::npos) {
                    float v(0);
                    in >> v;
                    camera()->frame()->setZoomSensitivity(v);
                } else if (keyword.find("transSens") != std::string::npos) {
                    float v(0);
                    in >> v;
                    camera()->frame()->setTranslationSensitivity(v);
                } else if (keyword.find("zoomsOnPivotPoint") != std::string::npos) {
                    bool v(false);
                    in >> v;
                    camera()->frame()->setZoomsOnPivotPoint(v);
                } else if (keyword.find("pivotPoint") != std::string::npos) {
                    vec3 p;
                    in >> p;
                    camera()->setPivotPoint(p);
                } else {
                    dummy = keyword.substr(0, keyword.find(':'));
                    LOG(WARNING) << "view property '" << dummy
                                 << "' is ignored (not recognized by this version of Mapple)";
                }
            }
        }
    }

    // now camera parameters are all up-to-date, enable updating the rendering
    easy3d::connect(&camera_->frame_modified, this, static_cast<void (PaintCanvas::*)(void)>(&PaintCanvas::update));

    update();
}


void PaintCanvas::showPrimitiveIDUnderMouse(bool b) {
    show_primitive_id_under_mouse_ = b;

    picked_face_index_ = -1;

    auto mesh = dynamic_cast<SurfaceMesh *>(currentModel());
    if (mesh) {
        auto drawable = mesh->renderer()->get_triangles_drawable("faces");
        drawable->set_highlight(b);
        drawable->set_highlight_range({-1, -1});
    }

    auto cloud = dynamic_cast<PointCloud *>(currentModel());
    if (cloud) {
        auto drawable = cloud->renderer()->get_points_drawable("vertices");
        drawable->set_highlight(b);
        drawable->set_highlight_range({-1, -1});
    }

    update();
}


void PaintCanvas::showPrimitivePropertyUnderMouse(bool b) {
    show_primitive_property_under_mouse_ = b;
    if (show_primitive_property_under_mouse_ && show_coordinates_under_mouse_)
        show_coordinates_under_mouse_ = false;

    picked_face_index_ = -1;

    auto mesh = dynamic_cast<SurfaceMesh *>(currentModel());
    if (mesh) {
        auto drawable = mesh->renderer()->get_triangles_drawable("faces");
        drawable->set_highlight(b);
        drawable->set_highlight_range({-1, -1});
    }

    auto cloud = dynamic_cast<PointCloud *>(currentModel());
    if (cloud) {
        auto drawable = cloud->renderer()->get_points_drawable("vertices");
        drawable->set_highlight(b);
        drawable->set_highlight_range({-1, -1});
    }

    update();
}


void PaintCanvas::showCoordinatesUnderMouse(bool b) {
    show_coordinates_under_mouse_ = b;
    if (show_primitive_property_under_mouse_ && show_coordinates_under_mouse_)
        show_primitive_property_under_mouse_ = false;

    QString coords("");
    if (show_coordinates_under_mouse_)
        coords = "XYZ = [-, -, -]";
    window_->statusBar()->showMessage(coords, 0);
}


void PaintCanvas::draw() {
    if (models_.empty())
        return;

    easy3d_debug_log_gl_error
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
        shadow()->draw(surfaces); easy3d_debug_log_gl_error
        return;
    } else if (transparency()) {
        transparency()->draw(surfaces); easy3d_debug_log_gl_error
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

    easy3d_debug_log_gl_error

    for (const auto m : models_) {
        if (!m->renderer()->is_visible())
            continue;

        // Let's check if edges and surfaces are both shown. If true, we
        // make the depth coordinates of the surface smaller, so that displaying
        // the mesh and the surface together does not cause Z-fighting.
        std::size_t count = 0;
        for (auto d : m->renderer()->lines_drawables()) {
            if (d->is_visible()) {
                d->draw(camera()); easy3d_debug_log_gl_error
                ++count;
            }
        }

        for (auto d : m->renderer()->points_drawables()) {
            if (d->is_visible())
                d->draw(camera()); easy3d_debug_log_gl_error
        }

        if (count > 0) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(0.5f, -0.0001f);
        }
        for (auto d : m->renderer()->triangles_drawables()) {
            if (d->is_visible())
                d->draw(camera()); easy3d_debug_log_gl_error
        }
        if (count > 0)
            glDisable(GL_POLYGON_OFFSET_FILL);
    }

    if (edl())
        edl()->end();
}
