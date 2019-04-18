/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <easy3d/viewer/viewer.h>

#include <thread>
#include <chrono>
#include <iostream>


#if !defined(_WIN32)
#  include <unistd.h>
#  include <sys/wait.h>
#endif


#include <easy3d/viewer/opengl.h>		// Initialize with glewInit() 
#include <3rd_party/glfw/include/GLFW/glfw3.h>	// Include glfw3.h after our OpenGL definitions

#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/viewer/opengl_info.h>
#include <easy3d/viewer/drawable.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/transform.h>
#include <easy3d/viewer/primitives.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/manipulated_camera_frame.h>
#include <easy3d/viewer/ambient_occlusion.h>
#include <easy3d/viewer/dual_depth_peeling.h>
#include <easy3d/viewer/average_color_blending.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/framebuffer_object.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/viewer/opengl_timer.h>

#include <easy3d/util/dialogs.h>
#include <easy3d/util/file.h>
#include <easy3d/util/timer.h>
#include <easy3d/fileio/point_cloud_io.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/fileio/ply_reader_writer.h>
#include <easy3d/fileio/point_cloud_io_ptx.h>


namespace easy3d {


	Viewer::Viewer(
		const std::string& title /* = "easy3d Viewer" */,
		int samples /* = 4 */,
		int gl_major /* = 3 */,
		int gl_minor /* = 2 */,
		bool full_screen /* = false */,
		bool resizable /* = true */,
		int depth_bits /* = 24 */,
		int stencil_bits /* = 8 */
	)
        : title_(title)
        , camera_(nullptr)
        , samples_(0)
        , full_screen_(full_screen)
        , process_events_(true)
        , gpu_timer_(nullptr)
        , show_corner_axes_(true)
        , axes_(nullptr)
        , show_pivot_point_(false)
        , model_idx_(-1)
	{
        // Avoid locale-related number parsing issues.
		setlocale(LC_NUMERIC, "C");

		glfwSetErrorCallback(
			[](int error, const char *descr) {
			if (error == GLFW_NOT_INITIALIZED)
                return;  // Ignore
			std::cerr << "GLFW error " << error << ": " << descr << std::endl;
		});

		if (!glfwInit())
			throw std::runtime_error("Could not initialize GLFW!");

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
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);	// 3.2+ only
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);			// 3.0+ only
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

        int w = 960;
        int h = 800;
		if (full_screen) {
			GLFWmonitor *monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode *mode = glfwGetVideoMode(monitor);
            w = mode->width;
            h = mode->height;
            window_ = glfwCreateWindow(w, h, title.c_str(), monitor, nullptr);
		}
		else {
            window_ = glfwCreateWindow(w, h, title.c_str(), nullptr, nullptr);
		}

		if (!window_) {
			glfwTerminate();
			throw std::runtime_error("Could not create an OpenGL " +
				std::to_string(gl_major) + "." + std::to_string(gl_minor) + " context!");
		}
        glfwSetWindowUserPointer(window_, this);

		glfwMakeContextCurrent(window_);
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        glfwSwapInterval(1); // Enable vsync

        // Load OpenGL and its extensions
        if (glewInit() != GLEW_OK) {
            glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
            throw std::runtime_error("Failed to load OpenGL and its extensions!");
        }

    #ifndef NDEBUG
        opengl::setup_gl_debug_callback();
    #endif

#if 1
        std::cout << "OpenGL version requested: " << gl_major << "." << gl_minor << std::endl;
        std::cout << "Supported OpenGL version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Supported GLSL version:   " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
#endif

        glGetIntegerv(GL_SAMPLES, &samples_);
        // warn the user if the requests were not satisfied
        if (samples > 0 && samples_ != samples) {
            if (samples_ == 0)
                printf("MSAA is not available with %i samples\n", samples);
            else {
                int max_num = 0;
                glGetIntegerv(GL_MAX_SAMPLES, &max_num);
                printf("MSAA is available with %i samples (%i requested, max support is %i)\n", samples_, samples, max_num);
            }
        }

        float xscale, yscale;
        glfwGetWindowContentScale(window_, &xscale, &yscale);
        dpi_scaling_ = static_cast<double>(xscale + yscale) * 0.5;

        gpu_timer_ = new OpenGLTimer(false);

        background_color_ = vec4(0.9f, 0.9f, 1.0f, 1.0f);

        camera_ = new Camera;

        int fw, fh;
        glfwGetFramebufferSize(window_, &fw, &fh);
        // needs to be executed once to ensure the viewer is initialized with correct viewer size
        callback_event_resize(fw, fh);

        mouse_x_ = mouse_y_ = 0;
        mouse_pressed_x_ = mouse_pressed_y_ = 0;
        button_ = -1;
        modifiers_ = 0;
        drag_active_ = false;
        process_events_ = true;

        setup_callbacks();

		std::cout << usage() << std::endl;

        /* Poll for events once before starting a potentially lengthy loading process.*/
        glfwPollEvents();
	}


	void Viewer::setup_callbacks() {
		glfwSetCursorPosCallback(window_, [](GLFWwindow *win, double x, double y)
		{
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;

            int w, h;
            glfwGetWindowSize(win, &w, &h);
			if (x >= 0 && x <= w && y >= 0 && y <= h)
				viewer->callback_event_cursor_pos(x, y);
			else if (viewer->drag_active_) {
				// Restrict the cursor to be within the client area during dragging
				if (x < 0) x = 0;	if (x > w) x = w;
				if (y < 0) y = 0;	if (y > h) y = h;
				glfwSetCursorPos(win, x, y);
			}
		});

        glfwSetMouseButtonCallback(window_, [](GLFWwindow *win, int button, int action, int modifiers)
        {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;
			viewer->callback_event_mouse_button(button, action, modifiers);
		});

        glfwSetKeyCallback(window_, [](GLFWwindow *win, int key, int scancode, int action, int mods)
        {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;
			(void)scancode;
			viewer->callback_event_keyboard(key, action, mods);
		});

        glfwSetCharCallback(window_, [](GLFWwindow *win, unsigned int codepoint)
        {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;
			viewer->callback_event_character(codepoint);
		});

        glfwSetDropCallback(window_, [](GLFWwindow *win, int count, const char **filenames)
        {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;
			viewer->callback_event_drop(count, filenames);
		});

        glfwSetScrollCallback(window_, [](GLFWwindow *win, double dx, double dy)
        {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;
			viewer->callback_event_scroll(dx, dy);
		});

        /* React to framebuffer size events -- includes window size events and also
         * catches things like dragging a window from a Retina-capable screen to a
         * normal screen on Mac OS X */
        glfwSetFramebufferSizeCallback(window_, [](GLFWwindow *win, int width, int height)
        {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;
			viewer->callback_event_resize(width, height);
		});

		// notify when the screen has lost focus (e.g. application switch)
        glfwSetWindowFocusCallback(window_, [](GLFWwindow *win, int focused)
        {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			viewer->focus_event(focused != 0);// true for focused
		});

        glfwSetWindowCloseCallback(window_, [](GLFWwindow *win)
        {
			glfwSetWindowShouldClose(win, true);
		});
	}


	bool Viewer::callback_event_cursor_pos(double x, double y) {
        int px = static_cast<int>(x);
        int py = static_cast<int>(y);
		try {
			int dx = px - mouse_x_;
            int dy = py - mouse_y_;
			mouse_x_ = px;
			mouse_y_ = py;
			if (drag_active_)
				return mouse_drag_event(px, py, dx, dy, button_, modifiers_);
            else
				return mouse_free_move_event(px, py, dx, dy, modifiers_);
		}
		catch (const std::exception &e) {
			std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
			return false;
		}
	}


	bool Viewer::callback_event_mouse_button(int button, int action, int modifiers) {
		try {
			if (action == GLFW_PRESS) {
				drag_active_ = true;
				button_ = button;
				modifiers_ = modifiers;
				mouse_pressed_x_ = mouse_x_;
				mouse_pressed_y_ = mouse_y_;
				return mouse_press_event(mouse_x_, mouse_y_, button, modifiers);
			}
			else if (action == GLFW_RELEASE) {
				drag_active_ = false;
				return mouse_release_event(mouse_x_, mouse_y_, button, modifiers);
			}
			else {
				drag_active_ = false;
				std::cout << "GLFW_REPEAT? Seems never happen" << std::endl;
				return false;
			}
		}
		catch (const std::exception &e) {
			std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
			return false;
		}
	}


	bool Viewer::callback_event_keyboard(int key, int action, int modifiers) {
		try {
			if (action == GLFW_PRESS || action == GLFW_REPEAT) {
				return key_press_event(key, modifiers);
			}
			else {
				return key_release_event(key, modifiers);
			}
		}
		catch (const std::exception &e) {
			std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
			return false;
		}
	}


	bool Viewer::callback_event_character(unsigned int codepoint) {
		try {
			return char_input_event(codepoint);
		}
		catch (const std::exception &e) {
			std::cerr << "Caught exception in event handler: " << e.what()
				<< std::endl;
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
			std::cerr << "Caught exception in event handler: " << e.what()
				<< std::endl;
			return false;
		}
	}


	bool Viewer::callback_event_scroll(double dx, double dy) {
		try {
			return mouse_scroll_event(mouse_x_, mouse_y_, static_cast<int>(dx), static_cast<int>(dy));
		}
		catch (const std::exception &e) {
			std::cerr << "Caught exception in event handler: " << e.what()
				<< std::endl;
			return false;
		}
	}


	void Viewer::callback_event_resize(int w, int h) {
		if (w == 0 && h == 0)
			return;       

        try {
            // camera is manipulated by the mouse, working in the screen coordinate system
            int win_w, win_h;
            glfwGetWindowSize(window_, &win_w, &win_h);
            camera_->setScreenWidthAndHeight(win_w, win_h);
            glViewport(0, 0, w, h);
            post_resize(w, h);
		}
		catch (const std::exception &e) {
			std::cerr << "Caught exception in event handler: " << e.what()
				<< std::endl;
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
	}


	void Viewer::cleanup() {
		// viewer may have already been destroyed by the user
		if (!window_)
			return;

        if (camera_) { delete camera_; camera_ = nullptr; }
        if (axes_) { delete axes_; axes_ = nullptr; }
 
		for (auto m : models_)
			delete m;
        models_.clear();

        if (gpu_timer_) { delete gpu_timer_; gpu_timer_ = nullptr; }

        ShaderManager::terminate();

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


    void Viewer::resize(int w, int h) {
#ifndef __APPLE__
        w = static_cast<int>(w * dpi_scaling());
        h = static_cast<int>(h * dpi_scaling());
#endif
        glfwSetWindowSize(window_, w, h);
    }


    int Viewer::width() const {
        int w, h;
        glfwGetWindowSize(window_, &w, &h);
        return w;
    }


    int Viewer::height() const {
        int w, h;
        glfwGetWindowSize(window_, &w, &h);
        return h;
    }


    void Viewer::update() const {
		glfwPostEmptyEvent();
	}


	bool Viewer::mouse_press_event(int x, int y, int button, int modifiers) {
        camera_->frame()->action_start();
		if (modifiers == GLFW_MOD_SHIFT && button == GLFW_MOUSE_BUTTON_RIGHT) {
			bool found = false;
            const vec3& p = point_under_pixel(x, y, found);
            if (found) {
				camera_->setPivotPoint(p);

                // show, but hide the visual hint of pivot point after \p delay milliseconds.
                show_pivot_point_ = true;
                const vec3& proj = camera()->projectedCoordinatesOf(camera()->pivotPoint());
                pivot_point_ = vec2(proj.x, proj.y);

                const int delay = 2000;
                Timer::single_shot(delay, [&]() {
                    show_pivot_point_ = false;
                    pivot_point_ = vec2(0, 0);
                    update();
                });
            }
            else {
				camera_->setPivotPoint(camera_->sceneCenter());
                show_pivot_point_ = false;
            }
		}
		return false;
	}


	bool Viewer::mouse_release_event(int x, int y, int button, int modifiers) {
		if (button == GLFW_MOUSE_BUTTON_LEFT && modifiers == GLFW_MOD_CONTROL) { // ZOOM_ON_REGION
			int xmin = std::min(mouse_pressed_x_, x);	int xmax = std::max(mouse_pressed_x_, x);
			int ymin = std::min(mouse_pressed_y_, y);	int ymax = std::max(mouse_pressed_y_, y);
			camera_->fitScreenRegion(xmin, ymin, xmax, ymax);
		}
		else
			camera_->frame()->action_end();

		button_ = -1;
		return false;
	}


	bool Viewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
		if (modifiers != GLFW_MOD_CONTROL) { // GLFW_MOD_CONTROL is reserved for zoom on region
			switch (button)
			{
			case GLFW_MOUSE_BUTTON_LEFT:
				camera_->frame()->action_rotate(x, y, dx, dy, camera_, modifiers == GLFW_MOD_ALT);
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				camera_->frame()->action_translate(x, y, dx, dy, camera_, modifiers == GLFW_MOD_ALT);
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				if (dy != 0)
					camera_->frame()->action_zoom(dy > 0 ? 1 : -1, camera_);
				break;
			}
		}

		return false;
	}


	bool Viewer::mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) {
        (void)x;
        (void)y;
        (void)dx;
        (void)dy;
        (void)modifiers;
        // highlight geometry primitives here
		return false;
	}


	bool Viewer::mouse_scroll_event(int x, int y, int dx, int dy) {
        (void)x;
        (void)y;
        (void)dx;
        camera_->frame()->action_zoom(dy, camera_);
		return false;
	}


    Model* Viewer::current_model() const {
        if (models_.empty())
            return nullptr;
        if (model_idx_ < models_.size())
            return models_[model_idx_];
        return nullptr;
    }

	bool Viewer::key_press_event(int key, int modifiers) {
		if (key == GLFW_KEY_A && modifiers == 0) {
			show_corner_axes_ = !show_corner_axes_;
		}
		else if (key == GLFW_KEY_C && modifiers == 0) {
            if (current_model())
                fit_screen(current_model());
		}
		else if (key == GLFW_KEY_F && modifiers == 0) {
            fit_screen();
		}
		else if (key == GLFW_KEY_LEFT && modifiers == 0) {
            float angle = static_cast<float>(1 * M_PI / 180.0); // turn left, 1 degrees each step
			camera_->frame()->action_turn(angle, camera_);
		}
		else if (key == GLFW_KEY_RIGHT && modifiers == 0) {
            float angle = static_cast<float>(1 * M_PI / 180.0); // turn right, 1 degrees each step
			camera_->frame()->action_turn(-angle, camera_);
		}
		else if (key == GLFW_KEY_UP && modifiers == 0) {	// move camera forward
			float step = 0.02f * camera_->sceneRadius();
			camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, 0.0, -step)));
		}
		else if (key == GLFW_KEY_DOWN && modifiers == 0) {// move camera backward
			float step = 0.02f * camera_->sceneRadius();
			camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, 0.0, step)));
		}
		else if (key == GLFW_KEY_UP && modifiers == GLFW_MOD_CONTROL) {	// move camera up
			float step = 0.02f * camera_->sceneRadius();
			camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, step, 0.0)));
		}
		else if (key == GLFW_KEY_DOWN && modifiers == GLFW_MOD_CONTROL) {	// move camera down 
			float step = 0.02f * camera_->sceneRadius();
			camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, -step, 0.0)));
		}
		else if (key == GLFW_KEY_M && modifiers == 0) {
			// NOTE: switching on/off MSAA in this way will affect all viewers because OpenGL 
			//       is a state machine. For multi-window applications, you have to call 
			//		 glDisable()/glEnable() before the individual draw functions.
			if (samples_ > 0) {
				if (glIsEnabled(GL_MULTISAMPLE)) {
					glDisable(GL_MULTISAMPLE);
					std::cout << title_ + ": MSAA disabled" << std::endl;
				}
				else {
					glEnable(GL_MULTISAMPLE);
					std::cout << title_ + ": MSAA enabled" << std::endl;
				}
			}
		}
		else if (key == GLFW_KEY_F1 && modifiers == 0)
			std::cout << usage() << std::endl;
		else if (key == GLFW_KEY_P && modifiers == 0) {
			if (camera_->type() == Camera::PERSPECTIVE)
				camera_->setType(Camera::ORTHOGRAPHIC);
			else
				camera_->setType(Camera::PERSPECTIVE);
		}
		else if (key == GLFW_KEY_SPACE && modifiers == 0) {
			// Aligns camera
			Frame frame;
			frame.setTranslation(camera_->pivotPoint());
			camera_->frame()->alignWithFrame(&frame, true);

			// Aligns frame
			//if (manipulatedFrame())
			//	manipulatedFrame()->alignWithFrame(camera_->frame());
		}
		else if (key == GLFW_KEY_O && modifiers == GLFW_MOD_CONTROL)
			open();
		else if (key == GLFW_KEY_S && modifiers == GLFW_MOD_CONTROL)
			save();

		else if (key == GLFW_KEY_MINUS && modifiers == 0) {
			for (auto m : models_) {
				for (auto d : m->points_drawables()) {
					float size = d->point_size() - 1.0f;
					if (size < 1)
						size = 1;
					d->set_point_size(size);
				}
			}
		}	
		else if (key == GLFW_KEY_EQUAL && modifiers == 0) {
			for (auto m : models_) {
				for (auto d : m->points_drawables()) {
					float size = d->point_size() + 1.0f;
					if (size > 20)
						size = 20;
					d->set_point_size(size);
				}
			}
		}

		else if (key == GLFW_KEY_MINUS && modifiers == GLFW_MOD_CONTROL)
			camera_->frame()->action_zoom(-1, camera_);
		else if (key == GLFW_KEY_EQUAL && modifiers == GLFW_MOD_CONTROL)
			camera_->frame()->action_zoom(1, camera_);

		else if (key == GLFW_KEY_COMMA && modifiers == 0) {
			if (models_.empty())
				model_idx_ = -1;
			else
				model_idx_ = int((model_idx_ - 1 + models_.size()) % models_.size());
			if (model_idx_ >= 0)
				std::cout << "current model: " << model_idx_ << ", " << models_[model_idx_]->name() << std::endl;
		}
		else if (key == GLFW_KEY_PERIOD && modifiers == 0) {
			if (models_.empty())
				model_idx_ = -1;
			else
				model_idx_ = int((model_idx_ + 1) % models_.size());
			if (model_idx_ >= 0)
				std::cout << "current model: " << model_idx_ << ", " << models_[model_idx_]->name() << std::endl;
		}
        else if (key == GLFW_KEY_DELETE && modifiers == 0) {
            if (current_model())
                delete_model(current_model());
        }
		else if (key == GLFW_KEY_W && modifiers == 0) {
            if (current_model()) {
                SurfaceMesh* m = dynamic_cast<SurfaceMesh*>(current_model());
				if (m) {
					LinesDrawable* wireframe = m->lines_drawable("wireframe");
					if (!wireframe) {
						wireframe = m->add_lines_drawable("wireframe");
						std::vector<unsigned int> indices;
						for (auto e : m->edges()) {
							SurfaceMesh::Vertex s = m->vertex(e, 0);
							SurfaceMesh::Vertex t = m->vertex(e, 1);
							indices.push_back(s.idx());
							indices.push_back(t.idx());
						}
                        auto points = m->get_vertex_property<vec3>("v:point");
                        wireframe->update_vertex_buffer(points.vector());
                        wireframe->update_index_buffer(indices);
                        wireframe->set_default_color(vec3(0.0f, 0.0f, 0.0f));
                        wireframe->set_per_vertex_color(false);
                        wireframe->set_visible(true);
					}
					else
						wireframe->set_visible(!wireframe->is_visible());
				}
			}
		}
        else if (key == GLFW_KEY_R && modifiers == 0) {
            ShaderManager::reload();
        }

        else if (key == GLFW_KEY_S && modifiers == 0) {
           snapshot();
           update();
        }

        else if (key == GLFW_KEY_F4 && modifiers == GLFW_MOD_ALT) {
            glfwSetWindowShouldClose(window_, true);
        }

		return false;
	}


	bool Viewer::key_release_event(int key, int modifiers) {
        (void)key;
        (void)modifiers;
		return false;
	}


	bool Viewer::char_input_event(unsigned int codepoint) {
        (void)codepoint;
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


	bool Viewer::drop_event(const std::vector<std::string> & filenames) {
		int count = 0;
		for (auto& name : filenames) {
			if (open(name))
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

        // NOTE: when dealing with OpenGL, we alway work in the highdpi screen space
#if defined(__APPLE__)
        glx = static_cast<int>(glx * dpi_scaling());
        gly = static_cast<int>(gly * dpi_scaling());
#endif
        float depth = std::numeric_limits<float>::max();
        glPixelStorei(GL_PACK_ALIGNMENT, 1);				easy3d_debug_gl_error;
        glReadPixels(glx, gly, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);					easy3d_debug_gl_error;
		found = depth < 1.0f;
		if (found) {
            vec3 point(float(x), float(y), depth);
            // The input to unprojectedCoordinatesOf() is defined in the screen coordinate system
            point = camera_->unprojectedCoordinatesOf(point);
			return point;
        }
        else
            return vec3();
	}


	inline double get_seconds() {
		return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
	}


	void Viewer::run() {
        // initialize before showing the window because it can be slow
        init();

        glfwShowWindow(window_);

		// TODO: make it member variable
		bool is_animating = false;

		try {
			// Rendering loop
			const int num_extra_frames = 5;
			const double animation_max_fps = 30;
			int frame_counter = 0;

			while (!glfwWindowShouldClose(window_)) {
				if (!glfwGetWindowAttrib(window_, GLFW_VISIBLE)) // not visible
					continue;

				double tic = get_seconds();
				pre_draw();

                gpu_timer_->start();
                draw();
                gpu_timer_->stop();
                gpu_time_ = gpu_timer_->time();

				post_draw();
				glfwSwapBuffers(window_);

				if (is_animating || frame_counter++ < num_extra_frames)
				{
					glfwPollEvents();
					// In microseconds
                    double duration = 1000000.* (get_seconds() - tic);
					const double min_duration = 1000000. / animation_max_fps;
					if (duration < min_duration)
                        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(min_duration - duration)));
				}
				else {
					/* Wait for mouse/keyboard or empty refresh events */
					glfwWaitEvents();
					frame_counter = 0;
				}
			}

			/* Process events once more */
			glfwPollEvents();
		}
		catch (const std::exception &e) {
			std::cerr << "Caught exception in main loop: " << e.what() << std::endl;
		}

		cleanup();
	}


	void Viewer::init() {
        glEnable(GL_DEPTH_TEST);

        glClearDepthf(1.0f);
        glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
	}


	std::string Viewer::usage() const {
		return std::string(
			"Easy3D viewer usage:												\n"
			"  F1:              Help											\n"
			"  Ctrl + O:        Open file										\n"
			"  Ctrl + S:        Save file										\n"
            "  Fn + Delete:     Delete current model                            \n"
            "  Left:            Orbit-rotate the camera							\n"
            "  Right:           Move up/down/left/right							\n"
            "  Alt + Left:      Orbit-rotate the camera (screen based)			\n"
            "  Alt + Right:     Move up/down/left/right (screen based)			\n"
			"  Middle/Wheel:    Zoom out/in										\n"
			"  Ctrl + '-'/'+':  Zoom out/in										\n"
            "  F:               Fit screen (all models)                         \n"
			"  C:               Fit screen (current model only)					\n"
            "  Shift + Right:   Set/unset anchor point							\n"
			"  P:               Toggle perspective/orthographic projection)		\n"
			"  A:               Toggle axes										\n"
			"  W:               Toggle wireframe								\n"
			"  < or >:          Switch between models							\n"
            "  M:               Toggle MSAA										\n"
            "  S:               Snapshot										\n"
        );
	}


	bool Viewer::open() {
        const std::vector<std::string> filetypes = {"*.ply", "*.obj", "*.off", "*.stl", "*.poly", "*.bin", "*.xyz", "*.bxyz", "*.las", "*.laz", "*.ptx"};
        const std::vector<std::string>& file_names = FileDialog::open(filetypes, true, "");

		int count = 0;
        for (const auto& file_name : file_names) {
            if (open(file_name))
                ++count;
        }
		return count > 0;
	}


	Model* Viewer::open(const std::string& file_name) {
        for (auto m : models_) {
            if (m->name() == file_name) {
                std::cout << "model alreaded loaded: \'" << file_name << std::endl;
                return nullptr;
            }
        }

        const std::string& ext = file::extension(file_name, true);
        bool is_ply_mesh = false;
        if (ext == "ply")
            is_ply_mesh = (io::PlyReader::num_faces(file_name) > 0);

        Model* model = nullptr;
        if ((ext == "ply" && is_ply_mesh) || ext == "obj" || ext == "off" || ext == "stl" || ext == "poly" || ext == "plg") { // mesh
            SurfaceMesh* mesh = SurfaceMeshIO::load(file_name);
            if (mesh) {
                model = mesh;
                create_drawables(mesh);
                std::cout << "mesh loaded. num faces: " << mesh->n_faces() << "; "
                    << "num vertices: " << mesh->n_vertices() << "; "
                    << "num edges: " << mesh->n_edges() << std::endl;
            }
        }
        else if (ext == "mesh" || ext == "meshb" || ext == "tet") { // cgraph
//            model = CGraphIO::read(name);
//            addModel(model, true, fit);
        }
        else { // point cloud
            if (ext == "ptx") {
                io::PointCloudIO_ptx serializer(file_name);
                PointCloud* cloud = nullptr;
                while ((cloud = serializer.load_next())) {
                    create_drawables(cloud);
                    add_model(cloud);
                    std::cout << "cloud loaded. num vertices: " << cloud->n_vertices() << std::endl;
                }
            }
            else {
                PointCloud* cloud = PointCloudIO::load(file_name);
                if (cloud) {
                    create_drawables(cloud);
                    model = cloud;
                    std::cout << "cloud loaded. num vertices: " << cloud->n_vertices() << std::endl;
                }
            }
        }

		if (model) {
			model->set_name(file_name);
			add_model(model);
			return model;
		}

		return nullptr;
	}


    void Viewer::create_drawables(Model* m) {
        if (dynamic_cast<PointCloud*>(m)) {
            PointCloud* cloud = dynamic_cast<PointCloud*>(m);
            // create points drawable
            auto points = cloud->get_vertex_property<vec3>("v:point");
            PointsDrawable* drawable = cloud->add_points_drawable("vertices");
            drawable->update_vertex_buffer(points.vector());
            auto normals = cloud->get_vertex_property<vec3>("v:normal");
            if (normals)
                drawable->update_normal_buffer(normals.vector());
            auto colors = cloud->get_vertex_property<vec3>("v:color");
            if (colors) {
                drawable->update_color_buffer(colors.vector());
                drawable->set_per_vertex_color(true);
            }
        }
        else if (dynamic_cast<SurfaceMesh*>(m)) {
            SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(m);
            TrianglesDrawable* surface = mesh->add_triangles_drawable("surface");
#if 1   // flat shading
            auto points = mesh->get_vertex_property<vec3>("v:point");
            auto colors = mesh->get_vertex_property<vec3>("v:color");

            std::vector<vec3> vertices, vertex_normals, vertex_colors;
            for (auto f : mesh->faces()) {
                // we assume convex polygonal faces and we render in triangles
                SurfaceMesh::Halfedge start = mesh->halfedge(f);
                SurfaceMesh::Halfedge cur = mesh->next_halfedge(mesh->next_halfedge(start));
                SurfaceMesh::Vertex va = mesh->to_vertex(start);
                const vec3& pa = points[va];
                while (cur != start) {
                    SurfaceMesh::Vertex vb = mesh->from_vertex(cur);
                    SurfaceMesh::Vertex vc = mesh->to_vertex(cur);
                    const vec3& pb = points[vb];
                    const vec3& pc = points[vc];
                    vertices.push_back(pa);
                    vertices.push_back(pb);
                    vertices.push_back(pc);

                    const vec3& n = geom::triangle_normal(pa, pb, pc);
                    vertex_normals.insert(vertex_normals.end(), 3, n);

                    if (colors) {
                        vertex_colors.push_back(colors[va]);
                        vertex_colors.push_back(colors[vb]);
                        vertex_colors.push_back(colors[vc]);
                    }
                    cur = mesh->next_halfedge(cur);
                }
            }
            surface->update_vertex_buffer(vertices);
            surface->update_normal_buffer(vertex_normals);
            if (colors)
                surface->update_color_buffer(vertex_colors);
            surface->release_index_buffer();
#else
            auto points = mesh->get_vertex_property<vec3>("v:point");
            surface->update_vertex_buffer(points.vector());
            auto colors = mesh->get_vertex_property<vec3>("v:color");
            if (colors)
                surface->update_color_buffer(colors.vector());

            auto normals = mesh->get_vertex_property<vec3>("v:normal");
            if (normals)
                 surface->update_normal_buffer(normals.vector());
            else {
                std::vector<vec3> normals;
                normals.reserve(mesh->n_vertices());
                for (auto v : mesh->vertices()) {
                    const vec3& n = mesh->compute_vertex_normal(v);
                    normals.push_back(n);
                }
                surface->update_normal_buffer(normals);
            }

            std::vector<unsigned int> indices;
            for (auto f : mesh->faces()) {
                // we assume convex polygonal faces and we render in triangles
                SurfaceMesh::Halfedge start = mesh->halfedge(f);
                SurfaceMesh::Halfedge cur = mesh->next_halfedge(mesh->next_halfedge(start));
                SurfaceMesh::Vertex va = mesh->to_vertex(start);
                while (cur != start) {
                    SurfaceMesh::Vertex vb = mesh->from_vertex(cur);
                    SurfaceMesh::Vertex vc = mesh->to_vertex(cur);
                    indices.push_back(static_cast<unsigned int>(va.idx()));
                    indices.push_back(static_cast<unsigned int>(vb.idx()));
                    indices.push_back(static_cast<unsigned int>(vc.idx()));
                    cur = mesh->next_halfedge(cur);
                }
            }
            surface->update_index_buffer(indices);
#endif
        }
    }


	void Viewer::add_model(Model* model) {
        if (!model)
            return;

        for (auto m : models_) {
            if (model == m) // model alreay added to the viewer
                return;
        }

        unsigned int num = model->n_vertices();
        if (num == 0) {
            std::cerr << "Warning: model does not have vertices. Only complete model can be added to the viewer." << std::endl;
            return;
        }

        if (model->points_drawables().empty() &&
            model->lines_drawables().empty() &&
            model->triangles_drawables().empty())
        {
            create_drawables(model);
        }

        Box3 box;
        if (dynamic_cast<PointCloud*>(model)) {
            PointCloud* cloud = dynamic_cast<PointCloud*>(model);
            auto points = cloud->get_vertex_property<vec3>("v:point");
            for (auto v : cloud->vertices())
                box.add_point(points[v]);
            model->set_bounding_box(box);
        }
        else if (dynamic_cast<SurfaceMesh*>(model)) {
            SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(model);
            auto points = mesh->get_vertex_property<vec3>("v:point");
            for (auto v : mesh->vertices())
                box.add_point(points[v]);
        }
        model->set_bounding_box(box);

        models_.push_back(model);
        model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current

        fit_screen();
	}


    void Viewer::delete_model(Model* model) {
        auto pos = std::find(models_.begin(), models_.end(), model);
        if (pos != models_.end()) {
            models_.erase(pos);
            delete model;
            model_idx_ = static_cast<int>(models_.size()) - 1; // make the last one current
            fit_screen();
        }
        else
            std::cerr << "no such model: " << model->name() << std::endl;
    }


    void Viewer::fit_screen(const easy3d::Model *model) {
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


	bool Viewer::save() const {
        if (!current_model()) {
            std::cerr << "no model exists" << std::endl;
            return false;
        }

        const std::vector<std::string> filetypes = {"*.ply", "*.obj", "*.off", "*.stl", "*.poly", "*.bin", "*.xyz", "*.bxyz", "*.las", "*.laz"};
        const Model* m = current_model();
        const std::string& file_name = FileDialog::save(filetypes, m->name());
        if (file_name.empty())
            return false;

        bool saved = false;
        if (dynamic_cast<const PointCloud*>(m)) {
            const PointCloud* cloud = dynamic_cast<const PointCloud*>(m);
            saved = PointCloudIO::save(file_name, cloud);
        }
        else if (dynamic_cast<const SurfaceMesh*>(m)) {
            const SurfaceMesh* mesh = dynamic_cast<const SurfaceMesh*>(m);
            saved = SurfaceMeshIO::save(file_name, mesh);
        }

        if (saved)
            std::cout << "file successfully saved" << std::endl;

        return saved;
	}


    bool Viewer::snapshot(bool bk_white /* = true*/) const {
        if (!current_model()) {
            std::cerr << "no model exists" << std::endl;
            return false;
        }

        const std::vector<std::string> filetypes = {"*.ppm *.tga *.bmp"};
        std::string file_name = file::replace_extension(current_model()->name(), "ppm");
        file_name = FileDialog::save(filetypes, file_name);
        if (file_name.empty())
            return false;

        const std::string& ext = file::extension(file_name, true);
        if (ext != "ppm" && ext != "tga" && ext != "bmp") {
            std::cerr << "snapshot format must be ppm, tga, or bmp" << std::endl;
            return false;
        }

        int w, h;
        glfwGetFramebufferSize(window_, &w, &h);
        FramebufferObject* fbo = new FramebufferObject(w, h, samples_);
        fbo->add_color_buffer();
        fbo->add_depth_buffer();

        fbo->bind();

        if (bk_white)
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        else
            glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        const_cast<Viewer*>(this)->draw();

        fbo->release();

        if (ext == "ppm")
            return fbo->snapshot_color_ppm(0, file_name);
        else if (ext == "tga")
            return fbo->snapshot_color_tga(0, file_name);
        else if (ext == "bmp")
            return fbo->snapshot_color_bmp(0, file_name);
        else {
            std::cerr << "snapshot format must be ppm, tga, or bmp" << std::endl;
            return false;
        }
    }


	void Viewer::draw_corner_axes() {
        ShaderProgram* program = ShaderManager::get_program("surface_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
            program = ShaderManager::create_program_from_files("surface_color", attributes);
        }
        if (!program)
            return;

        if (!axes_) {
            float base = 0.5f;   // the cylinder length, relative to the allowed region
            float head = 0.2f;   // the cone length, relative to the allowed region
            std::vector<vec3> points, normals, colors;
            opengl::prepare_cylinder(0.03, 10, vec3(0, 0, 0), vec3(base, 0, 0), vec3(1, 0, 0), points, normals, colors);
            opengl::prepare_cylinder(0.03, 10, vec3(0, 0, 0), vec3(0, base, 0), vec3(0, 1, 0), points, normals, colors);
            opengl::prepare_cylinder(0.03, 10, vec3(0, 0, 0), vec3(0, 0, base), vec3(0, 0, 1), points, normals, colors);
            opengl::prepare_cone(0.06, 20, vec3(base, 0, 0), vec3(base + head, 0, 0), vec3(1, 0, 0), points, normals, colors);
            opengl::prepare_cone(0.06, 20, vec3(0, base, 0), vec3(0, base + head, 0), vec3(0, 1, 0), points, normals, colors);
            opengl::prepare_cone(0.06, 20, vec3(0, 0, base), vec3(0, 0, base + head), vec3(0, 0, 1), points, normals, colors);
            opengl::prepare_sphere(vec3(0, 0, 0), 0.06, 20, 20, vec3(0, 1, 1), points, normals, colors);
            axes_ = new TrianglesDrawable("corner_axes");
            axes_->update_vertex_buffer(points);
            axes_->update_normal_buffer(normals);
            axes_->update_color_buffer(colors);
            axes_->set_per_vertex_color(true);
        }

        // The viewport and the scissor are changed to fit the lower left corner.
        int viewport[4], scissor[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_SCISSOR_BOX, scissor);

        static int corner_frame_size = static_cast<int>(100 * dpi_scaling());
        glViewport(0, 0, corner_frame_size, corner_frame_size);
        glScissor(0, 0, corner_frame_size, corner_frame_size);

        // To make the axis appear over other objects: reserve a tiny bit of the
        // front depth range. NOTE: do remember to restore it later.
        glDepthRangef(0, 0.001f);

        const mat4& proj = transform::ortho(-1, 1, -1, 1, -1, 1);
        const mat4& view = camera_->orientation().inverse().matrix();
        const mat4& MVP = proj * view;

        // camera position is defined in world coordinate system.
        const vec3& wCamPos = camera_->position();
        // it can also be computed as follows:
        //const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
        const mat4& MV = camera_->modelViewMatrix();
        const vec4& wLightPos = inverse(MV) * setting::light_position;

        program->bind();
        program->set_uniform("MVP", MVP);
        program->set_uniform("wLightPos", wLightPos);
        program->set_uniform("wCamPos", wCamPos);
        program->set_uniform("ssaoEnabled", false);
        program->set_uniform("per_vertex_color", true);
        axes_->draw(false);
        program->release();

        // restore
        glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glDepthRangef(0.0f, 1.0f);
	}


	void Viewer::pre_draw() {
		glfwMakeContextCurrent(window_);
		glClearColor(background_color_[0], background_color_[1], background_color_[2], 1.0f);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}


	void Viewer::post_draw() {
		// Visual hints: axis, camera, grid...
		if (show_corner_axes_)
			draw_corner_axes();

        if (show_pivot_point_) {

            ShaderProgram* program = ShaderManager::get_program("lines_color");
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes;
                attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
                attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
                program = ShaderManager::create_program_from_files("lines_color", attributes);
            }
            if (!program)
                return;

#if defined(__APPLE__)
            const float size = 10;
#else
            const float size = static_cast<float>(10 * dpi_scaling());
#endif
            LinesDrawable drawable("pivotpoint");
            std::vector<vec3> points = {
                vec3(pivot_point_.x - size, pivot_point_.y, 0.5f), vec3(pivot_point_.x + size, pivot_point_.y, 0.5f),
                vec3(pivot_point_.x, pivot_point_.y - size, 0.5f), vec3(pivot_point_.x, pivot_point_.y + size, 0.5f)
            };
            drawable.update_vertex_buffer(points);

            const mat4& proj = transform::ortho(0.0f, static_cast<float>(width()), static_cast<float>(height()), 0.0f, 0.0f, -1.0f);
            glDisable(GL_DEPTH_TEST);   // always on top
            program->bind();
            program->set_uniform("MVP", proj);
            program->set_uniform("per_vertex_color", false);
            program->set_uniform("default_color", vec3(0.0f, 0.0f, 1.0f));
            drawable.draw();
            program->release();
            glEnable(GL_DEPTH_TEST);   // restore
        }
	}


	void Viewer::draw() {
        if (models_.empty())
            return;

        // Let's check if wireframe and surfaces are both shown. If true, we
        // make the depth coordinates of the surface smaller, so that displaying
        // the mesh and the surface together does not cause Z-fighting.
        std::size_t count = 0;
        for (auto m : models_) {
            if (!m->is_visible())
                continue;
            for (auto d : m->lines_drawables()) {
                if (d->is_visible())
                    ++count;
            }
        }
        if (count > 0) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(0.5f, -0.0001f);
        }

        const mat4& MVP = camera_->modelViewProjectionMatrix();
        // camera position is defined in world coordinate system.
        const vec3& wCamPos = camera_->position();
        // it can also be computed as follows:
        //const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
        const mat4& MV = camera_->modelViewMatrix();
        const vec4& wLightPos = inverse(MV) * setting::light_position;

        ShaderProgram* program = ShaderManager::get_program("surface_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
            program = ShaderManager::create_program_from_files("surface_color", attributes);
        }
        if (program) {
            program->bind();
            program->set_uniform("MVP", MVP);
            program->set_uniform("wLightPos", wLightPos);
            program->set_uniform("wCamPos", wCamPos);
            program->set_uniform("ssaoEnabled", false);
            for (std::size_t idx = 0; idx < models_.size(); ++idx) {
                Model* m = models_[idx];
                if (!m->is_visible())
                    continue;
                for (auto d : m->triangles_drawables()) {
                    if (d->is_visible()) {
                        program->set_uniform("per_vertex_color", d->per_vertex_color() && d->color_buffer());
                        program->set_uniform("default_color", idx == model_idx_ ? d->default_color() : vec3(0.8f, 0.8f, 0.8f));
                        d->draw(false);
                    }
                }
            }
            program->release();
        }

        if (count > 0)
            glDisable(GL_POLYGON_OFFSET_FILL);

        program = ShaderManager::get_program("lines_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("lines_color", attributes);
        }
        if (program) {
            program->bind();
            program->set_uniform("MVP", MVP);
            for (auto m : models_) {
                if (!m->is_visible())
                    continue;
                for (auto d : m->lines_drawables()) {
                    if (d->is_visible()) {
                        program->set_uniform("per_vertex_color", d->per_vertex_color() && d->color_buffer());
                        program->set_uniform("default_color", d->default_color());
                        d->draw(false);
                    }
                }
            }
            program->release();
        }

        program = ShaderManager::get_program("points_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
            program = ShaderManager::create_program_from_files("points_color", attributes);
        }
        if (program) {
            program->bind();
            program->set_uniform("MVP", MVP);
            program->set_uniform("wLightPos", wLightPos);
            program->set_uniform("wCamPos", wCamPos);
            program->set_uniform("ssaoEnabled", false);
            for (auto m : models_) {
                if (!m->is_visible())
                    continue;
                for (auto d : m->points_drawables()) {
                    if (d->is_visible()) {
                        program->set_uniform("lighting", d->normal_buffer());
                        program->set_uniform("per_vertex_color", d->per_vertex_color() && d->color_buffer());
                        program->set_uniform("default_color", d->default_color());
                        d->draw(false);
                    }
                }
            }
            program->release();
        }
	}


}
