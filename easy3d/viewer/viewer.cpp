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

#include <easy3d/viewer/viewer.h>

#include <chrono>
#include <iostream>

#include <easy3d/renderer/opengl.h>				// for gl functions
#include <3rd_party/glfw/include/GLFW/glfw3.h>  // for glfw functions

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl_util.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/text_renderer.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/graph_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/poly_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/timer.h>
#include <easy3d/util/string.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/setting.h>


namespace easy3d {


    Viewer::Viewer(
            const std::string &title /* = "Easy3D Viewer" */,
            int samples /* = 4 */,
            int gl_major /* = 3 */,
            int gl_minor /* = 2 */,
            bool full_screen /* = false */,
            bool resizable /* = true */,
            int depth_bits /* = 24 */,
            int stencil_bits /* = 8 */,
            int width /* = 960 */,
            int height /* = 800 */
    )
        : window_(nullptr)
        , should_exit_(false)
        , dpi_scaling_(1.0)
        , title_(title)
        , camera_(nullptr)
        , is_animating_(false)
        , samples_(0)
        , full_screen_(full_screen)
        , background_color_(setting::background_color)
        , process_events_(true)
        , texter_(nullptr)
        , pressed_button_(-1)
        , modifiers_(-1)
        , drag_active_(false)
        , mouse_current_x_(0)
        , mouse_current_y_(0)
        , mouse_pressed_x_(0)
        , mouse_pressed_y_(0)
        , pressed_key_(-1)
        , show_pivot_point_(false)
        , show_frame_rate_(false)
        , drawable_axes_(nullptr)
        , show_camera_path_(false)
        , model_idx_(-1)
    {
        // Avoid locale-related number parsing issues.
        setlocale(LC_NUMERIC, "C");

        animation_func_ = nullptr;

        // Initialize logging (if it has not been initialized yet)
        if (!logging::is_initialized())
            logging::initialize(false, true, true, false, "default", 9);

        // create and setup window
        window_ = create_window(title, samples, gl_major, gl_minor, full_screen, resizable,
                                depth_bits, stencil_bits, width, height);
        setup_callbacks(window_);

        // create and set up the camera
        camera_ = new Camera;
        camera_->setType(Camera::PERSPECTIVE);
        camera_->setUpVector(vec3(0, 0, 1)); // Z pointing up
        camera_->setViewDirection(vec3(-1, 0, 0)); // X pointing out
        camera_->showEntireScene();

        easy3d::connect(&camera_->frame_modified, this, &Viewer::update);

        kfi_ = new KeyFrameInterpolator(camera_->frame());
        easy3d::connect(&kfi_->interpolation_stopped, this, &Viewer::update);

        sprintf(gpu_time_, "fps: ?? (?? ms/frame)");

        /* Poll for events once before starting a potentially lengthy loading process.*/
        glfwPollEvents();

        usage_string_ =
                " ------------------------------------------------------------------\n"
                " Easy3D viewer usage:                                              \n"
                " ------------------------------------------------------------------\n"
                "  F1:                  Help                                        \n"
                "  F2:                  Toggle animation (if available)             \n"
                "  Ctrl + 'o':          Open file                                   \n"
                "  Ctrl + 's':          Save file                                   \n"
                "  Fn + Delete:         Delete current model                        \n"
                "  '<' or '>':          Switch between models                       \n"
                "  's':                 Snapshot                                    \n"
                "  'p':                 Toggle perspective/orthographic projection) \n"
                "  'a':                 Toggle axes									\n"
                "  Ctrl + 'f':          Toggle frame rate                           \n"
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
                "  Alt + 'k':           Add key frame to the camera path            \n"
                "  Alt + 'd':           Delete the camera path                      \n"
                "  Ctrl + 'k':          Play the camera path                        \n"
                "  Ctrl + 't':          Toggle camera path                          \n"
                "  'f':                 Fit screen (all models)                     \n"
                "  'c':                 Fit screen (current model only)             \n"
                "  'z' + Left click:    Zoom to target point on model               \n"
                "  'z' + Right click:   Zoom o fit screen                           \n"
                "  Shift + Left click:  Define a target point on model              \n"
                "  Shift + Right click: Undefine the target point (if any) on model \n"
                "  Ctrl + Left drag:    Zoom to fit region                          \n"
                "  '-'/'=':             Decrease/Increase point size                \n"
                "  '{'/'}':             Decrease/Increase line width                \n"
                "  'b':                 Toggle borders								\n"
                "  'e':                 Toggle edges							    \n"
                "  'v':                 Toggle vertices                             \n"
                "  'm':                 Toggle smooth shading (for SurfaceMesh)     \n"
                "  'd':                 Print model info (drawables, properties)    \n"
                " ------------------------------------------------------------------\n";
    }


    GLFWwindow *Viewer::create_window(
            const std::string &title,
            int samples,
            int gl_major,
            int gl_minor,
            bool full_screen,
            bool resizable,
            int depth_bits,
            int stencil_bits,
            int width,
            int height) {
        glfwSetErrorCallback(
                [](int error, const char *desc) {
                    LOG(ERROR) << "GLFW error " << error << ": " << desc;
                });

        if (!glfwInit()) {
            LOG(ERROR) << "could not initialize GLFW!";
            throw std::runtime_error("could not initialize GLFW!");
        }

        glfwSetTime(0);

        // Reset the hints, allowing viewers to have different hints.
        glfwDefaultWindowHints();

        glfwWindowHint(GLFW_SAMPLES, samples);

        glfwWindowHint(GLFW_STENCIL_BITS, stencil_bits);
        glfwWindowHint(GLFW_DEPTH_BITS, depth_bits);

        /* Request a forward compatible OpenGL glMajor.glMinor core profile context.
           Default value is an OpenGL 3.2 core profile context. */
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_minor);

#ifdef __APPLE__
        // The only OpenGL 3.x and 4.x contexts currently supported by macOS are
        // forward-compatible, core profile contexts.
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
        if (gl_major >= 3) {
            if (gl_minor >= 2)
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	// 3.2+ only
            if (gl_minor >= 0)
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);			// 3.0+ only
        }
#endif

        // make the whole window transparent
        //glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GL_TRUE : GL_FALSE);

#ifndef NDEBUG
        const std::string title_str = title + " - Debug Version";
#else
        const std::string title_str = title;
#endif

        GLFWwindow *window = nullptr;
        if (full_screen) {
            GLFWmonitor *monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            width = mode->width;
            height = mode->height;
            window = glfwCreateWindow(width, height, title_str.c_str(), monitor, nullptr);
        } else {
            window = glfwCreateWindow(width, height, title_str.c_str(), nullptr, nullptr);
        }

        if (!window) {
            glfwTerminate();
            std::string error_message = "could not create an OpenGL "
                                        + std::to_string(gl_major) + "." + std::to_string(gl_minor) + " context!";
            LOG(ERROR) << error_message;
            throw std::runtime_error(error_message);
        }
        glfwSetWindowUserPointer(window, this);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        // Enable vsync
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // Load OpenGL and its extensions
        if (!OpenglUtil::init()) {
            LOG(ERROR) << "failed to load OpenGL and its extensions!";
            throw std::runtime_error("failed to load OpenGL and its extensions!");
        }

#ifndef NDEBUG
        opengl::setup_gl_debug_callback();
#endif

        VLOG(1) << "OpenGL vendor: " << glGetString(GL_VENDOR);
        VLOG(1) << "OpenGL renderer: " << glGetString(GL_RENDERER);
        VLOG(1) << "OpenGL version requested: " << gl_major << "." << gl_minor;
        VLOG(1) << "OpenGL version received: " << glGetString(GL_VERSION);
        VLOG(1) << "GLSL version received: " << glGetString(GL_SHADING_LANGUAGE_VERSION);

        glGetIntegerv(GL_SAMPLES, &samples_);
        int max_num = 0;
        glGetIntegerv(GL_MAX_SAMPLES, &max_num);

        // warn the user if the requests were not satisfied
        if (samples > 0 && samples_ != samples) {
            if (samples_ == 0)
                LOG(WARNING) << "MSAA is not available (" << samples << " samples requested)";
            else
                LOG(WARNING) << "MSAA is available with " << samples_ << " samples (" << samples
                             << " requested but max support is " << max_num << ")";
        } else
            VLOG(1) << "samples received: " << samples_ << " (" << samples << " requested, max support is "
                      << max_num << ")";

        // the request viewer size might not be satisfied, thus we query the actual size
        glfwGetWindowSize(window, &width_, &height_);
        VLOG(1) << "viewer size (width/height): " << width_ << "/" << height_;

        float xscale(0), yscale(0);
        glfwGetWindowContentScale(window, &xscale, &yscale);
        LOG_IF(xscale != yscale, ERROR) << "window content scale in X and Y directions not identical (X: " << xscale << ", Y: " << yscale << ")";
        dpi_scaling_ = static_cast<float>(xscale + yscale) * 0.5f;
        VLOG(1) << "DPI scaling: " << dpi_scaling();

        return window;
    }


    void Viewer::setup_callbacks(GLFWwindow *window) {
        glfwSetWindowRefreshCallback(window, [](GLFWwindow *win) {
            auto viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(win));
            viewer->pre_draw();
            viewer->draw();
            viewer->post_draw();
            glfwSwapBuffers(win);
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow *win, double x, double y) {
            auto viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;

            int w = viewer->width();
            int h = viewer->height();
            if (x >= 0 && x <= w && y >= 0 && y <= h)
                viewer->callback_event_cursor_pos(x, y);
            else if (viewer->drag_active_) {
                // Restrict the cursor to be within the client area during dragging
                if (x < 0) x = 0;
                if (x > w) x = w;
                if (y < 0) y = 0;
                if (y > h) y = h;
                glfwSetCursorPos(win, x, y);
            }
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow *win, int button, int action, int modifiers) {
            auto viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            viewer->callback_event_mouse_button(button, action, modifiers);
        });

        glfwSetKeyCallback(window, [](GLFWwindow *win, int key, int scancode, int action, int mods) {
            auto viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            (void) scancode;
            viewer->callback_event_keyboard(key, action, mods);
        });

        glfwSetCharCallback(window, [](GLFWwindow *win, unsigned int codepoint) {
            auto viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            viewer->callback_event_character(codepoint);
        });

        glfwSetDropCallback(window, [](GLFWwindow *win, int count, const char **filenames) {
            auto viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            viewer->callback_event_drop(count, filenames);
        });

        glfwSetScrollCallback(window, [](GLFWwindow *win, double dx, double dy) {
            auto viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            viewer->callback_event_scroll(dx, dy);
        });

#if 0
        /* React to framebuffer size events -- includes window size events and also
         * catches things like dragging a window from a Retina-capable screen to a
         * normal screen on Mac OS X */
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *win, int width, int height) {
            auto viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            viewer->callback_event_resize(width, height);
        });
#else
        glfwSetWindowSizeCallback(window, [](GLFWwindow *win, int width, int height) {
            auto viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(win));
            if (!viewer->process_events_)
                return;
            viewer->callback_event_resize(width, height);
        });
#endif

        // notify when the screen has lost focus (e.g. application switch)
        glfwSetWindowFocusCallback(window, [](GLFWwindow *win, int focused) {
            auto viewer = reinterpret_cast<Viewer *>(glfwGetWindowUserPointer(win));
            viewer->focus_event(focused != 0);// true for focused
        });

        glfwSetWindowCloseCallback(window, [](GLFWwindow *win) {
            glfwSetWindowShouldClose(win, true);
        });
    }


    bool Viewer::callback_event_cursor_pos(double x, double y) {
        int px = static_cast<int>(x);
        int py = static_cast<int>(y);
        try {
            int dx = px - mouse_current_x_;
            int dy = py - mouse_current_y_;
            mouse_current_x_ = px;
            mouse_current_y_ = py;
            if (drag_active_)
                return mouse_drag_event(px, py, dx, dy, pressed_button_, modifiers_);
            else
                return mouse_free_move_event(px, py, dx, dy, modifiers_);
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool Viewer::callback_event_mouse_button(int button, int action, int modifiers) {
        try {
            if (action == GLFW_PRESS) {
                drag_active_ = true;
                pressed_button_ = button;
                modifiers_ = modifiers;
                mouse_pressed_x_ = mouse_current_x_;
                mouse_pressed_y_ = mouse_current_y_;
                return mouse_press_event(mouse_current_x_, mouse_current_y_, button, modifiers);
            } else if (action == GLFW_RELEASE) {
                drag_active_ = false;
                return mouse_release_event(mouse_current_x_, mouse_current_y_, button, modifiers);
            } else {
                drag_active_ = false;
                LOG(INFO) << "GLFW_REPEAT? Seems never happen";
                return false;
            }
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool Viewer::callback_event_keyboard(int key, int action, int modifiers) {
        try {
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                return key_press_event(key, modifiers);
            } else {
                return key_release_event(key, modifiers);
            }
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool Viewer::callback_event_character(unsigned int codepoint) {
        try {
            return char_input_event(codepoint);
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool Viewer::callback_event_drop(int count, const char **filenames) {
        try {
            std::vector<std::string> arg(count);
            for (int i = 0; i < count; ++i)
                arg[i] = filenames[i];
            return drop_event(arg);
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    bool Viewer::callback_event_scroll(double dx, double dy) {
        try {
            return mouse_scroll_event(mouse_current_x_, mouse_current_y_, static_cast<int>(dx), static_cast<int>(dy));
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
            return false;
        }
    }


    void Viewer::callback_event_resize(int w, int h) {
        width_ = w;
        height_ = h;

        try {
            // camera is manipulated by the mouse, working in the screen coordinate system
            // (different from the viewport or framebuffer size, which are in pixel coordinates)
            camera_->setScreenWidthAndHeight(width_, height_);
#if 0
            // https://www.glfw.org/docs/3.3/window_guide.html#window_size
            // Do not pass the window size to glViewport or other pixel-based OpenGL calls. The window size is in
            // screen coordinates, not pixels. Use the framebuffer size, which is in pixels, for pixel-based calls.
            glViewport(0, 0, w, h);
#else
            int fw, fh;
            glfwGetFramebufferSize(window_, &fw, &fh);
            glViewport(0, 0, fw, fh);
#endif
            // process user changes
            post_resize(width_, height_);
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in event handler: " << e.what();
        }
    }


    bool Viewer::focus_event(bool focused) {
        if (focused) {
            // ...
        }
        return false;
    }


    Viewer::~Viewer() {
        cleanup();
        LOG(INFO) << "viewer terminated. Bye!";
    }

    void Viewer::clear_scene() {
        for (auto m : models_) {
            delete m->renderer();
            delete m->manipulator();
            delete m;
        }
        models_.clear();

        for (auto d : drawables_)
            delete d;
        drawables_.clear();
    }

    void Viewer::cleanup() {
        // viewer may have already been destroyed by the user
        if (!window_)
            return;

        delete camera_;
        delete kfi_;
        delete drawable_axes_;
        delete texter_;

        clear_scene();

        ShaderManager::terminate();
        TextureManager::terminate();

        glfwDestroyWindow(window_);
        window_ = nullptr;
        glfwTerminate();
    }


    void Viewer::set_title(const std::string &title) {
        if (title != title_) {
            glfwSetWindowTitle(window_, title.c_str());
            title_ = title;
        }
    }


    void Viewer::set_position(int xpos, int ypos) {
        glfwSetWindowPos(window_, xpos, ypos);
    }


    void Viewer::resize(int w, int h) {
#ifndef __APPLE__
        w = static_cast<int>(w * dpi_scaling());
        h = static_cast<int>(h * dpi_scaling());
#endif
        glfwSetWindowSize(window_, w, h);
    }


	int Viewer::framebuffer_width() const {
		int w, h;
		glfwGetFramebufferSize(window_, &w, &h);
		return w;
	}


	int Viewer::framebuffer_height() const {
		int w, h;
		glfwGetFramebufferSize(window_, &w, &h);
		return h;
	}


	void Viewer::framebuffer_size(int& w, int& h) const {
		glfwGetFramebufferSize(window_, &w, &h);
	}


    void Viewer::update() const {
        glfwPostEmptyEvent();
    }


    bool Viewer::mouse_press_event(int x, int y, int button, int modifiers) {
        camera_->frame()->action_start();

        if (pressed_key_ == GLFW_KEY_Z || modifiers == GLFW_MOD_SHIFT) {
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                bool found = false;
                const vec3 &p = point_under_pixel(x, y, found);
                if (found) {
                    camera_->setPivotPoint(p);
                    // show, but hide the visual hint of pivot point after \p delay milliseconds.
                    show_pivot_point_ = true;
                    Timer<>::single_shot(10000, [&]() {
                        show_pivot_point_ = false;
                        update();
                    });

                    if (pressed_key_ == GLFW_KEY_Z &&  modifiers == 0) {
#if 1   // with animation
                        camera()->interpolateToLookAt(p);
#else   // without animation
                        const float coef = 0.1f;
                        const vec3& pos = coef * camera()->frame()->position() + (1.0f - coef)*p;
                        const quat& ori = camera()->frame()->orientation();
                        camera_->frame()->setPositionAndOrientation(pos, ori);
                        camera_->lookAt(p);
#endif
                    }
                } else {
                    camera_->setPivotPoint(camera_->sceneCenter());
                    show_pivot_point_ = false;
                }
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                camera_->setPivotPoint(camera_->sceneCenter());
                show_pivot_point_ = false;
                if (pressed_key_ == GLFW_KEY_Z &&  modifiers == 0) {
#if 1   // with animation
                    camera()->interpolateToFitScene();
#else   // without animation
                    camera()->showEntireScene();
#endif
                }
            }
        }

        return false;
    }


    bool Viewer::mouse_release_event(int x, int y, int button, int modifiers) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && modifiers == MODIF_CTRL) { // ZOOM_ON_REGION
            int xmin = std::min(mouse_pressed_x_, x);
            int xmax = std::max(mouse_pressed_x_, x);
            int ymin = std::min(mouse_pressed_y_, y);
            int ymax = std::max(mouse_pressed_y_, y);
            camera_->fitScreenRegion(xmin, ymin, xmax, ymax);
        } else
            camera_->frame()->action_end();

        pressed_button_ = -1;
        return false;
    }


    bool Viewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
        // control modifier is reserved for zooming on region
        if (modifiers != MODIF_CTRL) {
            auto axis = ManipulatedFrame::NONE;
            if (pressed_key_ == GLFW_KEY_X) axis = ManipulatedFrame::HORIZONTAL;
            else if (pressed_key_ == GLFW_KEY_Y) axis = ManipulatedFrame::VERTICAL;
            else if (pressed_key_ == GLFW_KEY_O) axis = ManipulatedFrame::ORTHOGONAL;
            switch (button) {
                case GLFW_MOUSE_BUTTON_LEFT:
                    camera_->frame()->action_rotate(x, y, dx, dy, camera_, axis);
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    camera_->frame()->action_translate(x, y, dx, dy, camera_, axis);
                    break;
                case GLFW_MOUSE_BUTTON_MIDDLE:
                    if (std::abs(dy) >= 1)
                        camera_->frame()->action_zoom(dy > 0 ? 1 : -1, camera_);
                    break;
                default:
                    break;
            }
        }

        return false;
    }


    bool Viewer::mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) {
        (void) x;
        (void) y;
        (void) dx;
        (void) dy;
        (void) modifiers;
        // highlight geometry primitives here
        return false;
    }


    bool Viewer::mouse_scroll_event(int x, int y, int dx, int dy) {
        (void) x;
        (void) y;
        (void) dx;
        camera_->frame()->action_zoom(dy, camera_);
        return false;
    }


    Model *Viewer::current_model() const {
        if (models_.empty())
            return nullptr;
        if (model_idx_ < models_.size())
            return models_[model_idx_];
        return nullptr;
    }

    bool Viewer::key_press_event(int key, int modifiers) {
        auto mod_funcs = commands_.find(Key(key));
        if (mod_funcs != commands_.end()) {
            auto func_model = mod_funcs->second.find(Modifier(modifiers));
            if (func_model != mod_funcs->second.end()) {
                auto func = func_model->second.first;
                auto model = func_model->second.second;
                return func(this, model);
            }
        }

        if (key == GLFW_KEY_F1 && modifiers == 0)
            std::cout << usage() << std::endl;
        else if (key == GLFW_KEY_F2 && modifiers == 0) {
            is_animating_ = !is_animating_;
            if (is_animating_ && !animation_func_)
                LOG(WARNING) << "animation is enabled but the animation function (i.e., animation_func_) is missing";
        }
        else if (key == GLFW_KEY_LEFT && modifiers == 0) {
            auto angle = static_cast<float>(1 * M_PI / 180.0); // turn left, 1 degrees each step
            camera_->frame()->action_turn(angle, camera_);
        } else if (key == GLFW_KEY_RIGHT && modifiers == 0) {
            auto angle = static_cast<float>(1 * M_PI / 180.0); // turn right, 1 degrees each step
            camera_->frame()->action_turn(-angle, camera_);
        } else if (key == GLFW_KEY_UP && modifiers == 0) {    // move camera forward
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, 0.0, -step)));
        } else if (key == GLFW_KEY_DOWN && modifiers == 0) {// move camera backward
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, 0.0, step)));
        } else if (key == GLFW_KEY_LEFT && modifiers == MODIF_CTRL) {    // move camera left
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(-step, 0.0, 0.0)));
        } else if (key == GLFW_KEY_RIGHT && modifiers == MODIF_CTRL) {    // move camera right
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(step, 0.0, 0.0)));
        } else if (key == GLFW_KEY_UP && modifiers == MODIF_CTRL) {    // move camera up
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, step, 0.0)));
        } else if (key == GLFW_KEY_DOWN && modifiers == MODIF_CTRL) {    // move camera down
            float step = 0.05f * camera_->sceneRadius();
            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, -step, 0.0)));
        } else if (key == GLFW_KEY_C && modifiers == MODIF_CTRL) {    // copy camera
            copy_view();
        } else if (key == GLFW_KEY_V && modifiers == MODIF_CTRL) {    // copy camera
            paste_view();
        }
        else if (key == GLFW_KEY_A && modifiers == 0) {
            if (drawable_axes_)
                drawable_axes_->set_visible(!drawable_axes_->is_visible());
        }
        else if (key == GLFW_KEY_F && modifiers == MODIF_CTRL)
            show_frame_rate_ = !show_frame_rate_;
        else if (key == GLFW_KEY_C && modifiers == 0) {
            if (current_model())
                fit_screen(current_model());
        } else if (key == GLFW_KEY_F && modifiers == 0) {
            fit_screen();
        } else if (key == GLFW_KEY_M && modifiers == 0) {
#if 0
            // NOTE: switching on/off MSAA in this way will affect all viewers because OpenGL
            //       is a state machine. For multi-window applications, you have to call
            //		 glDisable()/glEnable() before the individual draw functions.
            if (samples_ > 0) {
                if (glIsEnabled(GL_MULTISAMPLE)) {
                    glDisable(GL_MULTISAMPLE);
                    LOG(INFO) << title_ + ": MSAA disabled";
                }
                else {
                    glEnable(GL_MULTISAMPLE);
                    LOG(INFO) << title_ + ": MSAA enabled";
                }
            }
#else
            if (dynamic_cast<SurfaceMesh *>(current_model())) {
                auto drawables = current_model()->renderer()->triangles_drawables();
                for (auto d : drawables)
                    d->set_smooth_shading(!d->smooth_shading());
            }
#endif
        } else if (key == GLFW_KEY_P && modifiers == 0) {
            if (camera_->type() == Camera::PERSPECTIVE)
                camera_->setType(Camera::ORTHOGRAPHIC);
            else
                camera_->setType(Camera::PERSPECTIVE);
        } else if (key == GLFW_KEY_SPACE && modifiers == 0) {
            // Aligns camera
            Frame frame;
            frame.setTranslation(camera_->pivotPoint());
            camera_->frame()->alignWithFrame(&frame, true);

            // Aligns frame
            //if (manipulatedFrame())
            //	manipulatedFrame()->alignWithFrame(camera_->frame());
        } else if (key == GLFW_KEY_O && modifiers == MODIF_CTRL)
            open();
        else if (key == GLFW_KEY_S && modifiers == MODIF_CTRL)
            save();

        else if (key == GLFW_KEY_MINUS && modifiers == MODIF_CTRL)
            camera_->frame()->action_zoom(-1, camera_);
        else if (key == GLFW_KEY_EQUAL && modifiers == MODIF_CTRL)
            camera_->frame()->action_zoom(1, camera_);

        else if (key == GLFW_KEY_K && modifiers == GLFW_MOD_ALT) { // add key frame
            easy3d::Frame *frame = camera()->frame();
            kfi_->add_keyframe(*frame);
            LOG(INFO) << "key frame added to camera path";
        } else if (key == GLFW_KEY_D && modifiers == GLFW_MOD_ALT) { // delete path
            kfi_->delete_path();
            // update scene bounding box
            Box3 box;
            for (auto m : models_)
                box.grow(m->bounding_box());
            camera_->setSceneBoundingBox(box.min_point(), box.max_point());
            LOG(INFO) << "camera path deleted";
        } else if (key == GLFW_KEY_K && modifiers == MODIF_CTRL) { // play the path
            if (kfi_->is_interpolation_started())
                kfi_->stop_interpolation();
            else
                kfi_->start_interpolation();
        } else if (key == GLFW_KEY_T && modifiers == MODIF_CTRL) {
            show_camera_path_ = !show_camera_path_;
            if (show_camera_path_) {
                const std::size_t count = kfi_->number_of_keyframes();
                float radius = camera_->sceneRadius();
                for (std::size_t i=0; i<count; ++i) {
                    radius = std::max( radius,
                            distance(camera_->sceneCenter(), kfi_->keyframe(i).position())
                    );
                }
                camera_->setSceneRadius(radius);
            }
            else {
                Box3 box;
                for (auto m : models_)
                    box.grow(m->bounding_box());
                camera_->setSceneBoundingBox(box.min_point(), box.max_point());
            }
        } else if (key == GLFW_KEY_LEFT_BRACKET && modifiers == 0) {
            for (auto m : models_) {
                for (auto d : m->renderer()->lines_drawables()) {
                    float size = d->line_width() - 1.0f;
                    if (size < 1)
                        size = 1;
                    d->set_line_width(size);
                }
            }
        }
        else if (key == GLFW_KEY_RIGHT_BRACKET && modifiers == 0) {
            for (auto m : models_) {
                for (auto d : m->renderer()->lines_drawables()) {
                    float size = d->line_width() + 1.0f;
                    d->set_line_width(size);
                }
            }
        } else if (key == GLFW_KEY_MINUS && modifiers == 0) {
            for (auto m : models_) {
                for (auto d : m->renderer()->points_drawables()) {
                    float size = d->point_size() - 1.0f;
                    if (size < 1)
                        size = 1;
                    d->set_point_size(size);
                }
            }
        } else if (key == GLFW_KEY_EQUAL && modifiers == 0) {
            for (auto m : models_) {
                for (auto d : m->renderer()->points_drawables()) {
                    float size = d->point_size() + 1.0f;
                    d->set_point_size(size);
                }
            }
        } else if (key == GLFW_KEY_COMMA && modifiers == 0) {
            if (models_.empty())
                model_idx_ = -1;
            else
                model_idx_ = int((model_idx_ - 1 + models_.size()) % models_.size());
            if (model_idx_ >= 0) {
                fit_screen(models_[model_idx_]);
                std::cout << "current model: " << model_idx_ << ", " << models_[model_idx_]->name() << std::endl;
            }
        } else if (key == GLFW_KEY_PERIOD && modifiers == 0) {
            if (models_.empty())
                model_idx_ = -1;
            else
                model_idx_ = int((model_idx_ + 1) % models_.size());
            if (model_idx_ >= 0) {
                fit_screen(models_[model_idx_]);
                std::cout << "current model: " << model_idx_ << ", " << models_[model_idx_]->name() << std::endl;
            }
        } else if (key == GLFW_KEY_DELETE && modifiers == 0) {
            if (current_model())
                delete_model(current_model());
        } else if (key == GLFW_KEY_E && modifiers == 0) {
            if (current_model()) {
                auto *drawable = current_model()->renderer()->get_lines_drawable("edges");
                if (drawable)
					drawable->set_visible(!drawable->is_visible());
            }
        } else if (key == GLFW_KEY_V && modifiers == 0) {
            if (current_model()) {
                auto drawable = current_model()->renderer()->get_points_drawable("vertices");
                if (drawable)
					drawable->set_visible(!drawable->is_visible());
            }
        } else if (key == GLFW_KEY_B && modifiers == 0) {
            auto mesh = dynamic_cast<SurfaceMesh *>(current_model());
            if (mesh) {
                auto drawable = mesh->renderer()->get_lines_drawable("borders");
                if (drawable)
                    drawable->set_visible(!drawable->is_visible());
            }
        } else if (key == GLFW_KEY_L && modifiers == 0) { // locked vertices
            auto mesh = dynamic_cast<SurfaceMesh *>(current_model());
            if (mesh) {
                auto drawable = mesh->renderer()->get_points_drawable("locks");
                if (drawable)
                    drawable->set_visible(!drawable->is_visible());
            }
        } else if (key == GLFW_KEY_D && modifiers == 0) {
            if (current_model()) {
                auto &output = std::cout;

                output << "----------- " << file_system::simple_name(current_model()->name()) << " -----------\n";
                if (dynamic_cast<SurfaceMesh *>(current_model())) {
                    auto model = dynamic_cast<SurfaceMesh *>(current_model());
                    output << "model is a surface mesh. #face: " << std::to_string(model->n_faces())
                           << ", #vertex: " + std::to_string(model->n_vertices())
                           << ", #edge: " + std::to_string(model->n_edges()) << std::endl;
                } else if (dynamic_cast<PointCloud *>(current_model())) {
                    auto model = dynamic_cast<PointCloud *>(current_model());
                    output << "model is a point cloud. #vertex: " + std::to_string(model->n_vertices()) << std::endl;
                } else if (dynamic_cast<Graph *>(current_model())) {
                    auto model = dynamic_cast<Graph *>(current_model());
                    output << "model is a graph. #vertex: " + std::to_string(model->n_vertices())
                           << ", #edge: " + std::to_string(model->n_edges()) << std::endl;
                }

                if (!current_model()->renderer()->points_drawables().empty()) {
                    output << "points drawables:\n";
                    for (auto d : current_model()->renderer()->points_drawables())
                        d->buffer_stats(output);
                }
                if (!current_model()->renderer()->lines_drawables().empty()) {
                    output << "lines drawables:\n";
                    for (auto d : current_model()->renderer()->lines_drawables())
                        d->buffer_stats(output);
                }
                if (!current_model()->renderer()->triangles_drawables().empty()) {
                    output << "triangles drawables:\n";
                    for (auto d : current_model()->renderer()->triangles_drawables())
                        d->buffer_stats(output);
                }

                current_model()->property_stats(output);
            }
        } else if (key == GLFW_KEY_R && modifiers == 0) {
            // Reload the shader(s) - useful for writing/debugging shader code.
            ShaderManager::reload();
        } else if (key == GLFW_KEY_S && modifiers == 0) {
            snapshot();
        } else if (key == GLFW_KEY_F4 && modifiers == GLFW_MOD_ALT) {
            glfwSetWindowShouldClose(window_, true);
        }
        else
            pressed_key_ = key;

        return false;
    }


    bool Viewer::key_release_event(int key, int modifiers) {
        (void) key;
        (void) modifiers;
        pressed_key_ = -1;
        return false;
    }


    bool Viewer::char_input_event(unsigned int codepoint) {
        (void) codepoint;
        //switch (codepoint) {
        //case '-':	break;
        //case 'c':	break;
        //case 'C':	break;
        //case '0':	break;
        //default:
        //	return false;
        //}

        return false;
    }


    bool Viewer::drop_event(const std::vector<std::string> &filenames) {
        int count = 0;
        for (auto &name : filenames) {
            if (add_model(name))
                ++count;
        }

        if (count > 0) {
            model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current
            fit_screen();
            return true;
        }
        return false;
    }


    vec3 Viewer::point_under_pixel(int x, int y, bool &found) const {
        // GLFW (same as Qt) uses upper corner for its origin while GL uses the lower corner.
        int glx = x;
        int gly = height() - 1 - y;

        // NOTE: when dealing with OpenGL, we always work in the highdpi screen space
#if defined(__APPLE__)
        glx = static_cast<int>(static_cast<float>(glx) * dpi_scaling());
        gly = static_cast<int>(static_cast<float>(gly) * dpi_scaling());
#endif
        float depth = std::numeric_limits<float>::max();
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        easy3d_debug_log_gl_error
        glReadPixels(glx, gly, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
        easy3d_debug_log_gl_error
        found = depth < 1.0f;
        if (found) {
            vec3 point(static_cast<float>(x), static_cast<float>(y), depth);
            // The input to unprojectedCoordinatesOf() is defined in the screen coordinate system
            point = camera_->unprojectedCoordinatesOf(point);
            return point;
        } else
            return vec3(0);
    }


    int Viewer::run(bool see_all) {
        // initialize before showing the window because it can be slow
        init();

        // make sure scene fits the screen when the window appears
        if (see_all)
            fit_screen();

        // show the window
        glfwShowWindow(window_);

        try {   // main loop
            static int frame_counter = 0;
            double last_time = glfwGetTime();   // for frame rate counter

            while (!glfwWindowShouldClose(window_) && !should_exit_) {
                if (!glfwGetWindowAttrib(window_, GLFW_VISIBLE)) { // not visible
                    glfwWaitEvents();
                    continue;
                }

                if (show_frame_rate_) {
                    // Calculate ms/frame
                    double current_time = glfwGetTime();
                    ++frame_counter;
                    if (current_time - last_time >= 1.0f) {
                        sprintf(gpu_time_, "fps: %2.0f (%4.1f ms/frame)",
                                double(frame_counter) / (current_time - last_time), 1000.0 / double(frame_counter));
                        frame_counter = 0;
                        last_time = current_time;
                    }
                }

                pre_draw();
                draw();
                post_draw();
                glfwSwapBuffers(window_);

                // Don't call 'glfwPollEvents()' at the beginning of the main loop.
                // Reason: first frame needs time to complete.
                if (is_animating_ && animation_func_) {
                    glfwPollEvents();

                    // TODO: make framerate a parameter
                    if (!show_frame_rate_) {
                        static const int animation_fps = 30;
                        static const double interval = 1000.0 / (animation_fps + 5); // the extra 5 for adjusting
                        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(interval)));
                    }
                    animation_func_(this);
                }
                else if (show_frame_rate_)
                    glfwPollEvents();
                else
                    glfwWaitEvents();
            }

            /* Process events once more */
            glfwPollEvents();
            return EXIT_SUCCESS;
        }
        catch (const std::exception &e) {
            LOG(ERROR) << "Caught exception in main loop: " << e.what();
            cleanup();
            return EXIT_FAILURE;
        }
    }


    void Viewer::exit() {
        should_exit_ = true;
        update();
    }


    void Viewer::init() {
        //// back face culling?
        //glEnable(GL_CULL_FACE);
        //glCullFace(GL_BACK); // culls only the back faces

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glClearDepth(1.0f);
        glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);

        // camera is manipulated by the mouse, working in the screen coordinate system
        // (different from the viewport or framebuffer size, which are in pixel coordinates)
        camera_->setScreenWidthAndHeight(width_, height_);
        int fw, fh;
        glfwGetFramebufferSize(window_, &fw, &fh);
        glViewport(0, 0, fw, fh);

        // create TextRenderer renderer and load default fonts
        texter_ = new TextRenderer(dpi_scaling());
        texter_->add_font(resource::directory() + "/fonts/en_Earth-Normal.ttf");
        texter_->add_font(resource::directory() + "/fonts/en_Roboto-Medium.ttf");

        // print usage
        std::cout << usage() << std::endl;
    }


    void Viewer::set_animation(bool b) {
        is_animating_ = b;
        update();
    }


    bool Viewer::is_animating() const {
        return is_animating_ && animation_func_;
    }


    Model *Viewer::add_model(const std::string &file_path, bool create_default_drawables) {
        const std::string file_name = file_system::convert_to_native_style(file_path);
        for (auto m : models_) {
            if (m->name() == file_name) {
                LOG(WARNING) << "model has already been added to the viewer: " << file_name;
                return m;
            }
        }

        const std::string &ext = file_system::extension(file_name, true);
        bool is_ply_mesh = false;
        if (ext == "ply")
            is_ply_mesh = (io::PlyReader::num_instances(file_name, "face") > 0);

        Model *model = nullptr;
        if ((ext == "ply" && is_ply_mesh) || ext == "obj" || ext == "off" || ext == "stl" || ext == "sm" || ext == "geojson" || ext == "trilist") { // mesh
            model = SurfaceMeshIO::load(file_name);
        } else if (ext == "ply" && io::PlyReader::num_instances(file_name, "edge") > 0) {
            model = GraphIO::load(file_name);
        } else if (ext == "plm" || ext == "pm" || ext == "mesh") {
            model = PolyMeshIO::load(file_name);
        }
        else { // point cloud
            if (ext == "ptx") {
                io::PointCloudIO_ptx serializer(file_name);
                PointCloud *cloud = nullptr;
                while ((cloud = serializer.load_next())) {
                    model = add_model(cloud, create_default_drawables);
                    update();
                }
                return model;   // returns the last cloud in the file.
            } else
                model = PointCloudIO::load(file_name);
        }

        if (model) {
            model->set_name(file_name);
            add_model(model, create_default_drawables);
        }
        return model;
    }


    Model *Viewer::add_model(Model *model, bool create) {
        if (!model) {
            LOG(WARNING) << "model is NULL.";
            return nullptr;
        }
        for (auto m : models_) {
            if (model == m) {
                LOG(WARNING) << "model has already been added to the viewer: " << m->name();
                return nullptr;
            }
        }

        model->set_renderer(new Renderer(model, create));

        int pre_idx = model_idx_;
        models_.push_back(model);
        model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current

        if (model_idx_ != pre_idx) {
            if (model_idx_ >= 0)
                LOG(INFO) << "current model: " << model_idx_ << ", " << models_[model_idx_]->name();
        }
        return model;
    }


    bool Viewer::delete_model(Model *model) {
        if (!model) {
            LOG(WARNING) << "model is NULL.";
            return false;
        }

        auto pos = std::find(models_.begin(), models_.end(), model);
        if (pos != models_.end()) {
            int pre_idx = model_idx_;
            const std::string name = model->name();
            models_.erase(pos);
            delete model->renderer();
            delete model->manipulator();
            delete model;
            model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current
            LOG(INFO) << "model deleted: " << name;

            if (model_idx_ != pre_idx) {
                if (model_idx_ >= 0)
                    LOG(INFO) << "current model: " << model_idx_ << ", " << models_[model_idx_]->name();
            }
            return true;
        } else {
            LOG(WARNING) << "no such model: " << model->name();
            return false;
        }
    }


    bool Viewer::add_drawable(Drawable *drawable) {
        if (!drawable) {
            LOG(WARNING) << "drawable is NULL.";
            return false;
        }
        for (auto d : drawables_) {
            if (drawable == d) {
                LOG(WARNING) << "drawable has already been added to the viewer.";
                return false;
            }
        }

        drawables_.push_back(drawable);
        return true;
    }


    bool Viewer::delete_drawable(Drawable *drawable) {
        if (!drawable) {
            LOG(WARNING) << "drawable is NULL";
            return false;
        }

        auto pos = std::find(drawables_.begin(), drawables_.end(), drawable);
        if (pos != drawables_.end()) {
            drawables_.erase(pos);
            delete drawable;
            return true;
        } else {
            LOG(WARNING) << "no such drawable: " << drawable->name();
            return false;
        }
    }


    void Viewer::fit_screen(const easy3d::Model *model) {
        if (!model && models_.empty() && drawables_.empty()) {
            camera_->showEntireScene();
            return;
        }

        auto visual_box = [](const Model *m) -> Box3 {
            Box3 box = m->bounding_box();
            for (auto d : m->renderer()->points_drawables()) box.grow(d->bounding_box());
            for (auto d : m->renderer()->lines_drawables()) box.grow(d->bounding_box());
            for (auto d : m->renderer()->triangles_drawables()) box.grow(d->bounding_box());
            return box;
        };

        Box3 box;
        if (model)
            box = visual_box(model);
        else {
            for (auto m : models_)
                box.grow(visual_box(m));
            for (auto d : drawables_)
                box.grow(d->bounding_box());
        }

        if (box.is_valid()) {
            camera_->setSceneBoundingBox(box.min_point(), box.max_point());
            camera_->showEntireScene();
            update();
        }
    }


    bool Viewer::open() {
        const std::string title("Please choose a file");
        const std::string &default_path = resource::directory() + "/data/";
        const std::vector<std::string> &filters = {
                "Surface Mesh (*.obj *.ply *.off *.stl *.sm *.geojson *.trilist)", "*.obj *.ply *.off *.stl *.sm *.geojson *.trilist",
                "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx)", "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg *.ptx",
                "Polyhedral Mesh (*.plm *.pm *.mesh)", "*.plm *.pm *.mesh",
                "Graph (*.ply)", "*.ply",
                "All Files (*.*)", "*"
        };
        const std::vector<std::string> &file_names = dialog::open(title, default_path, filters, true);

        int count = 0;
        for (const auto &file_name : file_names) {
            if (add_model(file_name))
                ++count;
        }

        if (count > 0) {
            fit_screen();
            return true;
        }
        return false;
    }


    bool Viewer::save() const {
        const Model *m = current_model();
        if (!m) {
            LOG(ERROR) << "no model exists";
            return false;
        }

        const std::string &title = "Please choose a file name";
        const std::vector<std::string> &filters = {
                "Surface Mesh (*.obj *.ply *.off *.stl *.sm)", "*.obj *.ply *.off *.stl *.sm",
                "Point Cloud (*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg)",
                "*.bin *.ply *.xyz *.bxyz *.las *.laz *.vg *.bvg",
                "Polyhedral Mesh (*.plm *.pm *.mesh)", "*.plm *.pm *.mesh",
                "Graph (*.ply)", "*.ply",
                "All Files (*.*)", "*"
        };

        std::string default_file_name = m->name();
        if (file_system::extension(default_file_name).empty()) // no extension?
            default_file_name += ".ply"; // default to ply

        const bool warn_overwrite = true;
        const std::string &file_name = dialog::save(title, default_file_name, filters, warn_overwrite);
        if (file_name.empty())
            return false;

        bool saved = false;
        if (dynamic_cast<const PointCloud *>(m))
            saved = PointCloudIO::save(file_name, dynamic_cast<const PointCloud *>(m));
        else if (dynamic_cast<const SurfaceMesh *>(m))
            saved = SurfaceMeshIO::save(file_name, dynamic_cast<const SurfaceMesh *>(m));
        else if (dynamic_cast<const Graph *>(m))
            saved = GraphIO::save(file_name, dynamic_cast<const Graph *>(m));

        if (saved)
            LOG(INFO) << "file successfully saved";

        return saved;
    }


    bool Viewer::snapshot() const {
        const std::string &title = "Please choose a file name";
        std::string default_file_name("untitled.png");
        if (current_model())
            default_file_name = file_system::replace_extension(current_model()->name(), "png");
        const std::vector<std::string> &filters = {
                "Image Files (*.png *.jpg *.bmp *.tga)", "*.png *.jpg *.bmp *.tga",
                "All Files (*.*)", "*"
        };

        const bool warn_overwrite = true;
        const std::string &file_name = dialog::save(title, default_file_name, filters, warn_overwrite);
        if (file_name.empty())
            return false;

        const std::string &ext = file_system::extension(file_name, true);
        if (ext != "png" && ext != "jpg" && ext != "bmp" && ext != "tga") {
            LOG(ERROR) << "snapshot format must be png, jpg, bmp, or tga";
            return false;
        }

        return snapshot(file_name, 1.0f, samples_);
    }


    void Viewer::draw_corner_axes() const {
        ShaderProgram *program = ShaderManager::get_program("surface/surface");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                    ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                    ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"),
                    ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"),
                    ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal")
            };
            program = ShaderManager::create_program_from_files("surface/surface", attributes);
        }
        if (!program)
            return;

        if (!drawable_axes_) {
            float base = 0.5f;   // the cylinder length, relative to the allowed region
            float head = 0.2f;   // the cone length, relative to the allowed region
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
            const_cast<Viewer*>(this)->drawable_axes_ = new TrianglesDrawable("corner_axes");
            drawable_axes_->update_vertex_buffer(points);
            drawable_axes_->update_normal_buffer(normals);
            drawable_axes_->update_color_buffer(colors);
            drawable_axes_->set_property_coloring(State::VERTEX);
        }
        if (!drawable_axes_->is_visible())
            return;

        // The viewport is changed to fit the lower left corner.
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        static int corner_frame_size = static_cast<int>(100 * dpi_scaling());
        glViewport(0, 0, corner_frame_size, corner_frame_size);

        // To make the axis appear over other objects: reserve a tiny bit of the
        // front depth range. NOTE: do remember to restore it later.
        glDepthRange(0, 0.01f);

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
                ->set_uniform("use_texture", false);
        drawable_axes_->gl_draw();
        program->release();

        // restore the viewport
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glDepthRange(0.0f, 1.0f);
    }


    void Viewer::pre_draw() {
        glfwMakeContextCurrent(window_);
        glClearColor(background_color_[0], background_color_[1], background_color_[2], 1.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }


    void Viewer::post_draw() {
        // draw the Easy3D logo and GPU time
        if (texter_ && texter_->num_fonts() >=2) {
            const float font_size = 15.0f;
            const float offset = 20.0f * dpi_scaling();
            texter_->draw("Easy3D", offset, offset, font_size, 0);

            if (show_frame_rate_)
                texter_->draw(gpu_time_, offset, 50.0f * dpi_scaling(), 16, 1);
        }

        // shown only when it is not animating
        if (show_camera_path_ && !kfi_->is_interpolation_started()) {
            kfi_->draw_path(camera_, 2.0f);
            kfi_->draw_cameras(camera_, camera()->sceneRadius() * 0.05f);
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

#if defined(__APPLE__)
            const float size = 10;
#else
            const float size = static_cast<float>(10 * dpi_scaling());
#endif
            LinesDrawable drawable("pivot_point");
            const vec3 &pivot = camera()->projectedCoordinatesOf(camera()->pivotPoint());
            const std::vector<vec3> points = {
                    vec3(pivot.x - size, pivot.y, 0.5f), vec3(pivot.x + size, pivot.y, 0.5f),
                    vec3(pivot.x, pivot.y - size, 0.5f), vec3(pivot.x, pivot.y + size, 0.5f)
            };
            drawable.update_vertex_buffer(points);

            const mat4 &proj = transform::ortho(0.0f, static_cast<float>(width()), static_cast<float>(height()), 0.0f,
                                                0.0f, -1.0f);
            glDisable(GL_DEPTH_TEST);   // always on top
            program->bind();
            program->set_uniform("MVP", proj);
            program->set_uniform("per_vertex_color", false);
            program->set_uniform("default_color", vec4(0.0f, 0.0f, 1.0f, 1.0f));
            drawable.gl_draw();
            program->release();
            glEnable(GL_DEPTH_TEST);   // restore
        }

        // ------------- draw the picking region with transparency  ---------------

        if (pressed_button_ == GLFW_MOUSE_BUTTON_LEFT && modifiers_ == MODIF_CTRL) {
            const Rect rect(
                static_cast<float>(mouse_pressed_x_), 
                static_cast<float>(mouse_current_x_), 
                static_cast<float>(mouse_pressed_y_), 
                static_cast<float>(mouse_current_y_)
            );
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

        draw_corner_axes();
    }


    void Viewer::draw_face_labels(Model *model, TextRenderer *texter, int font_id, const vec3 &color) const {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        const auto &points = mesh->points();
        for (auto f : mesh->faces()) {
            int count = 0;
            vec3 c(0, 0, 0);
            for (auto v : mesh->vertices(f)) {
                c += points[v.idx()];
                ++count;
            }
            c /= count;

            const vec3 p = camera()->projectedCoordinatesOf(c);
            const float x = p.x * dpi_scaling_;
            const float y = (static_cast<float>(height()) - 1.0f - p.y) * dpi_scaling_;

            float depth = 1.0f;
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            easy3d_debug_log_gl_error
            glReadPixels(static_cast<int>(x), static_cast<int>(y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
            easy3d_debug_log_gl_error
            if (depth < 1.0 && p.z < 1.0 && std::abs(depth - p.z) < 0.001)
                texter->draw(std::to_string(f.idx()), x, p.y * dpi_scaling_, 16, font_id, color);
        }
    }


    void Viewer::draw_vertex_labels(Model *model, TextRenderer *texter, int font_id, const vec3 &color) const {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        const auto &points = mesh->points();
        for (std::size_t id = 0; id < points.size(); ++id) {
            const vec3& v = points[id];
            const vec3 p = camera()->projectedCoordinatesOf(v);
            const float x = p.x * dpi_scaling_;
            const float y = (static_cast<float>(height()) - 1.0f - p.y) * dpi_scaling_;

            float depth = 1.0f;
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            easy3d_debug_log_gl_error
            glReadPixels(static_cast<int>(x), static_cast<int>(y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
            easy3d_debug_log_gl_error
            if (depth < 1.0 && p.z < 1.0 && std::abs(depth - p.z) < 0.001)
                texter->draw(std::to_string(id), x, p.y * dpi_scaling_, 16, font_id, color);
        }
    }


    void Viewer::copy_view() {
        const vec3 pos = camera()->position();
        const quat q = camera()->orientation();

        std::stringstream stream;
        stream << pos[0] << ' '
                << pos[1] << ' '
                << pos[2] << ' '
                << q[0] << ' '
                << q[1] << ' '
                << q[2] << ' '
                << q[3];
        glfwSetClipboardString(window_, stream.str().c_str());
    }


    void Viewer::paste_view() {
        // get the camera parameters from clipboard string
        const std::string str = glfwGetClipboardString(window_);
        std::vector<std::string> list;
        string::split(str, ' ', list, true);
        if (list.size() != 7) {
            LOG(WARNING) << "camera not available in clipboard";
            return;
        }

        vec3 pos;
        for (int i = 0; i < 3; ++i)
            pos[i] = std::stof(list[i]);
        quat orient;
        for (int i = 0; i < 4; ++i)
            orient[i] = std::stof(list[i + 3]);

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


    void Viewer::draw() const {
//        glEnable(GL_SCISSOR_TEST);
//        int scissor[4];
//        glGetIntegerv(GL_SCISSOR_BOX, scissor);
//        glScissor(scissor[0], scissor[1], scissor[2] * 0.5f, scissor[3]);
//        current_model()->renderer()->get_lines_drawable("edges")->set_visible(false);
//        draw();
//        glScissor(scissor[2] * 0.5f, scissor[1], scissor[2] * 0.5f, scissor[3]);
//        current_model()->renderer()->get_lines_drawable("edges")->set_visible(true);
//        draw();
//        glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);

        for (const auto m : models_) {
            if (!m->renderer()->is_visible())
                continue;

            // Let's check if edges and surfaces are both shown. If true, we
            // make the depth coordinates of the surface smaller, so that displaying
            // the mesh and the surface together does not cause Z-fighting.
            std::size_t count = 0;
            for (auto d : m->renderer()->lines_drawables()) {
                if (d->is_visible()) {
                    d->draw(camera());
                    easy3d_debug_log_gl_error
                    ++count;
                }
            }

            for (auto d : m->renderer()->points_drawables()) {
                if (d->is_visible())
                    d->draw(camera());
                easy3d_debug_log_gl_error
            }

            if (count > 0) {
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(0.5f, -0.0001f);
            }
            for (auto d : m->renderer()->triangles_drawables()) {
                if (d->is_visible())
                    d->draw(camera());
                easy3d_debug_log_gl_error
            }
            if (count > 0)
                glDisable(GL_POLYGON_OFFSET_FILL);
        }

        for (auto d : drawables_) {
            if (d->is_visible())
                d->draw(camera());
        }

#if 0 // draw face labels and vertex labels
        if (dynamic_cast<SurfaceMesh *>(current_model()) && texter_ && texter_->num_fonts() > 1) {
            draw_face_labels(current_model(), texter_, 1, vec3(0, 0, 1));
            draw_vertex_labels(current_model(), texter_, 1, vec3(0, 1, 0));
        }
#endif
    }

}
