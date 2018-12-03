/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of EasyGUI: software for processing and rendering
*   meshes and point clouds.
*
*	EasyGUI is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	EasyGUI is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "viewer.h"

#include <thread>
#include <chrono>
#include <iostream>


#if !defined(_WIN32)
#  include <unistd.h>
#  include <sys/wait.h>
#endif


#include <3rd_party/glew/include/GL/glew.h>		// Initialize with glewInit() 
#include <3rd_party/glfw/include/GLFW/glfw3.h>	// Include glfw3.h after our OpenGL definitions

#include <easy3d/core/camera.h>
#include <easy3d/core/manipulated_camera_frame.h>
#include <easy3d/model/surface_mesh.h>
#include <easy3d/model/point_cloud.h>
#include <easy3d/core/drawable.h>
#include <easy3d/core/shader_program.h>
#include <easy3d/core/shader_code.h>
#include <easy3d/core/opengl_error.h>
#include <easy3d/core/file_dialog.h>
#include <easy3d/core/transform.h>


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
		, full_screen_(full_screen)
		, process_events_(true)
		, samples_(0)
		, show_corner_axes_(true)
		, width_(1280)	// default width
		, height_(960)	// default height
		, pressed_key_(GLFW_KEY_UNKNOWN)
		, axes_ (nullptr)
		, camera_(nullptr)
		, lines_program_(nullptr)
		, surface_program_(nullptr)
		, model_idx_(-1)
	{
#if !defined(_WIN32)
		/* Avoid locale-related number parsing issues */
		setlocale(LC_NUMERIC, "C");
#endif

		glfwSetErrorCallback(
			[](int error, const char *descr) {
			if (error == GLFW_NOT_INITIALIZED)
				return; /* Ignore */
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

		if (full_screen) {
			GLFWmonitor *monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode *mode = glfwGetVideoMode(monitor);
			window_ = glfwCreateWindow(mode->width, mode->height, title.c_str(), monitor, nullptr);
		}
		else {
			window_ = glfwCreateWindow(width_, height_, title.c_str(), nullptr, nullptr);
		}

		if (!window_) {
			glfwTerminate();
			throw std::runtime_error("Could not create an OpenGL " +
				std::to_string(gl_major) + "." + std::to_string(gl_minor) + " context!");
		}

		glfwMakeContextCurrent(window_);

		// Load OpenGL and its extensions
		if (glewInit() != GLEW_OK) {
			glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
			throw std::runtime_error("Failed to load OpenGL and its extensions!");
		}

		glfwSwapInterval(1); // Enable vsync

#if defined(DEBUG) || defined(_DEBUG)
        std::cout << "OpenGL Version requested: " << gl_major << "." << gl_minor << std::endl;
		int major = glfwGetWindowAttrib(window_, GLFW_CONTEXT_VERSION_MAJOR);
		int minor = glfwGetWindowAttrib(window_, GLFW_CONTEXT_VERSION_MINOR);
		int rev = glfwGetWindowAttrib(window_, GLFW_CONTEXT_REVISION);
        std::cout << "OpenGL version received:  " << major << "." << minor << "." << rev << std::endl;
        std::cout << "Supported OpenGL:         " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Supported GLSL:           " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
#endif

        std::string vender(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
        if (vender.find("Intel") != std::string::npos) {
			std::cerr << "Detected Intel HD Graphics card, disabling MSAA as a precaution .." << std::endl;
		}

		glGetIntegerv(GL_SAMPLES, &samples_);
		// warn the user if the expected request was not satisfied
		if (samples > 0 && samples_ != samples) {
			if (samples_ == 0)
				printf("MSAA is not available with %i samples\n", samples);
			else {
				int max_num = 0;
				glGetIntegerv(GL_MAX_SAMPLES, &max_num);
				printf("MSAA is available with %i samples (%i requested, max support is %i)\n", samples_, samples, max_num);
			}
		}

		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

		setup_callbacks();

		background_color_[0] = background_color_[1] = background_color_[2] = 0.3f;
		mouse_x_ = mouse_y_ = 0;
		mouse_pressed_x_ = mouse_pressed_y_ = 0;
		button_ = -1;
		pressed_key_ = GLFW_KEY_UNKNOWN;
		modifiers_ = 0;
		drag_active_ = false;
		process_events_ = true;

		glfwSetWindowUserPointer(window_, this);

#if defined(__APPLE__)
		/* Poll for events once before starting a potentially lengthy loading process.*/
		glfwPollEvents();
#endif
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

		glfwSetMouseButtonCallback(window_, [](GLFWwindow *win, int button, int action, int modifiers) {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;
			viewer->callback_event_mouse_button(button, action, modifiers);
		});

		glfwSetKeyCallback(window_, [](GLFWwindow *win, int key, int scancode, int action, int mods) {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;
			(void)scancode;
			viewer->callback_event_keyboard(key, action, mods);
		});

		glfwSetCharCallback(window_, [](GLFWwindow *win, unsigned int codepoint) {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;
			viewer->callback_event_character(codepoint);
		});

		glfwSetDropCallback(window_, [](GLFWwindow *win, int count, const char **filenames) {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;
			viewer->callback_event_drop(count, filenames);
		});

		glfwSetScrollCallback(window_, [](GLFWwindow *win, double dx, double dy) {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;
			viewer->callback_event_scroll(dx, dy);
		});

		/* React to framebuffer size events -- includes window size events
		   and also catches things like dragging a window from a Retina-capable
		   screen to a normal screen on Mac OS X */
		glfwSetFramebufferSizeCallback(window_, [](GLFWwindow *win, int width, int height) {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			if (!viewer->process_events_)
				return;
			viewer->callback_event_resize(width, height);
		});

		// notify when the screen has lost focus (e.g. application switch)
		glfwSetWindowFocusCallback(window_, [](GLFWwindow *win, int focused) {
			Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(win));
			viewer->focus_event(focused != 0);// true for focused
		});

		glfwSetWindowCloseCallback(window_, [](GLFWwindow *win) {
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
			width_ = w;
			height_ = h;
			glViewport(0, 0, w, h);
			camera_->setScreenWidthAndHeight(w, h);
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

		if (camera_) {
			delete camera_;
			camera_ = nullptr;
		}

		if (lines_program_) {
			delete lines_program_;
			lines_program_ = nullptr;
		}

		if (surface_program_) {
			delete surface_program_;
			surface_program_ = nullptr;
		}

		if (axes_) {
			delete axes_;
			axes_ = nullptr;
		}

		for (auto m : models_)
			delete m;

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
		glfwSetWindowSize(window_, w, h);
	}


	void Viewer::update() const {
		glfwPostEmptyEvent();
	}


	bool Viewer::mouse_press_event(int x, int y, int button, int modifiers) {
		camera_->frame()->action_start();
		if (modifiers == GLFW_MOD_SHIFT && button == GLFW_MOUSE_BUTTON_RIGHT) {
			bool found = false;
			vec3 p = point_under_pixel(x, y, found);
			if (found)
				camera_->setPivotPoint(p);
			else
				camera_->setPivotPoint(camera_->sceneCenter());
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
		// highlight geometry primitives here
		return false;
	}


	bool Viewer::mouse_scroll_event(int x, int y, int dx, int dy) {
		camera_->frame()->action_zoom(dy, camera_);
		return false;
	}


	bool Viewer::key_press_event(int key, int modifiers) {
		if (key == GLFW_KEY_A && modifiers == 0) {
			show_corner_axes_ = !show_corner_axes_;
		}		
		else if (key == GLFW_KEY_C && modifiers == 0) {
			if (!models_.empty() && model_idx_ > 0 && model_idx_ < models_.size()) {
				const Box3& box = models_[model_idx_]->bounding_box();
				camera_->setSceneBoundingBox(box.min(), box.max());
				camera_->showEntireScene();
			}
		}		
		else if (key == GLFW_KEY_F && modifiers == 0) {
			if (!models_.empty()){
				Box3 box;
				for (auto m : models_)
					box.add_box(m->bounding_box());
				camera_->setSceneBoundingBox(box.min(), box.max());
				camera_->showEntireScene();
			}
		}
		else if (key == GLFW_KEY_LEFT && modifiers == 0) {
			float angle = float(1 * M_PI / 180.0f); // turn left, 1 degrees each step
			camera_->frame()->action_turn(angle, camera_);
		}
		else if (key == GLFW_KEY_RIGHT && modifiers == 0) {
			float angle = float(1 * M_PI / 180.0f); // turn right, 1 degrees each step
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
			// NOTE: switching on/off MSAA in this way only works for a single-window 
			//       application, because OpenGL is a state machine. For multi-window
			//       applications, you have to call glDisable()/glEnable() before the
			//       individual draw functions.
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
		else if (key == GLFW_KEY_MINUS && modifiers == GLFW_MOD_CONTROL)
			camera_->frame()->action_zoom(-1, camera_);
		else if (key == GLFW_KEY_EQUAL && modifiers == GLFW_MOD_CONTROL)
			camera_->frame()->action_zoom(1, camera_);

		else if (key == GLFW_KEY_COMMA && modifiers == 0) {
			if (models_.empty())
				model_idx_ = -1;
			else
				model_idx_ = (model_idx_ - 1) % models_.size();
		}
		else if (key == GLFW_KEY_PERIOD && modifiers == 0) {
			if (models_.empty())
				model_idx_ = -1;
			else
				model_idx_ = (model_idx_ + 1) % models_.size();
		}
		else if (key == GLFW_KEY_W && modifiers == 0) {
			if (model_idx_ < models_.size()) {
				Surface_mesh* m = dynamic_cast<Surface_mesh*>(models_[model_idx_]);
				if (m) {
					LinesDrawable* wireframe = m->line_drawable("wireframe");
					if (!wireframe) {
						wireframe = m->add_line_drawable("wireframe");
						std::vector<unsigned int> indices;
						for (auto e : m->edges()) {
							Surface_mesh::Vertex s = m->vertex(e, 0);
							Surface_mesh::Vertex t = m->vertex(e, 1);
							indices.push_back(s.idx());
							indices.push_back(t.idx());
						}
						auto points = m->get_vertex_property<vec3>("v:point");
						wireframe->update_vertex_buffer(points.vector());
						wireframe->update_index_buffer(indices);
					}
					else
						wireframe->set_visible(!wireframe->is_visible());
				}
			}
		}

		pressed_key_ = key;

		return false;
	}


	bool Viewer::key_release_event(int key, int modifiers) {
		pressed_key_ = GLFW_KEY_UNKNOWN;
		return false;
	}


	bool Viewer::char_input_event(unsigned int codepoint) {
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
		for (auto& name : filenames) {
			open_mesh(name);
		}
		return false;
	}


	vec3 Viewer::point_under_pixel(int x, int y, bool &found) const {
		float depth = std::numeric_limits<float>::max();
		// Qt uses upper corner for its origin while GL uses the lower corner.
		glReadPixels(x, height_ - 1 - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);	mpl_debug_gl_error;
		found = depth < 1.0f;
		if (found) {
			vec3 point(float(x), float(y), depth);
			point = camera_->unprojectedCoordinatesOf(point);
			return point;
		}
		return vec3();
	}


	inline double get_seconds() {
		return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
	}


	void Viewer::run() {
		init();

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
				draw();
				post_draw();
				glfwSwapBuffers(window_);

				if (is_animating || frame_counter++ < num_extra_frames)
				{
					glfwPollEvents();
					// In microseconds
					double duration = 1000000.*(get_seconds() - tic);
					const double min_duration = 1000000. / animation_max_fps;
					if (duration < min_duration)
						std::this_thread::sleep_for(std::chrono::microseconds((int)(min_duration - duration)));
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
		camera_ = new Camera;
		camera_->setSceneRadius(1.0f);
		camera_->setSceneCenter(vec3(0, 0, 0));
		camera_->setScreenWidthAndHeight(width_, height_);

		// seems depth test is disabled by default
		glEnable(GL_DEPTH_TEST);
		glfwShowWindow(window_);	

		// create shader programs
		lines_program_ = new ShaderProgram("line_color");
		if (lines_program_->load_shader_from_code(ShaderProgram::VERTEX, easy3d::shadercode::lines_color_vert) &&
			lines_program_->load_shader_from_code(ShaderProgram::FRAGMENT, easy3d::shadercode::lines_color_frag))
		{
			lines_program_->set_attrib_name(ShaderProgram::POSITION, "vtx_position");
			lines_program_->set_attrib_name(ShaderProgram::COLOR, "vtx_color");
			lines_program_->link_program();
		}
		else {
			std::cerr << "failed creating shader program for lines" << std::endl;
			delete surface_program_;
			surface_program_ = nullptr;
		}

		surface_program_ = new ShaderProgram("surface_color");
#if 0 // using files
		// Liangliang: Weird relative path doesn't work!!!!!! I have been using this for many years.
		const std::string& vert_file = "../../resources/shaders/surface_color.vert";
		const std::string& frag_file = "../../resources/shaders/surface_color.frag";
		if (surface_program_->load_shader_from_file(ShaderProgram::VERTEX, vert_file) &&
			surface_program_->load_shader_from_file(ShaderProgram::FRAGMENT, frag_file))
#else
		if (surface_program_->load_shader_from_code(ShaderProgram::VERTEX, easy3d::shadercode::surface_color_vert) &&
			surface_program_->load_shader_from_code(ShaderProgram::FRAGMENT, easy3d::shadercode::surface_color_frag))
#endif
		{
			surface_program_->set_attrib_name(ShaderProgram::POSITION, "vtx_position");
			surface_program_->set_attrib_name(ShaderProgram::COLOR, "vtx_color");
			surface_program_->link_program();
		}
		else {
			std::cerr << "failed creating shader program for surfaces" << std::endl;
			delete surface_program_;
			surface_program_ = nullptr;
		}
	}


	std::string Viewer::usage() const {
		return std::string(R"(Easy3D viewer usage:
  F1:              Help
  Ctrl + O:        Open file
  Ctrl + S:        Save file
  Left:            Rotate scene
  Right:           Translate scene
  Alt + Left:      Rotate scene (screen based)
  Alt + Right:     Translate scene (screen based)
  Middle/Wheel:    Zoom out/in
  Ctrl + '-'/'+':  Zoom out/in
  F:               Fit screen (entire scene/all models)     
  C:               Fit screen (current model only)
  Shift + Right:   Set/unset pivot point
  P:               Toggle perspective/orthographic projection)
  A:               Toggle axes
  W:               Toggle wireframe
  ',' or '.':      Switch between models
)"
);
		//   ;       Toggle vertex labels
		//   :       Toggle face labels)
	}


	bool Viewer::open() {
		std::vector< std::pair<std::string, std::string> > filetypes = {
			{"obj", "Wavefront Mesh"},
			{"off", "Object File Format"},
			{"ply", "PLY Mesh or Point Cloud"}
		};
		const std::vector<std::string>& files = easy3d::file_dialog(filetypes, false, true);
		int count = 0;
		Box3 box;
		for (const auto& f : files) {
			Surface_mesh* m = open_mesh(f);
			if (m) {
				// fit screen
				Box3 b;
				auto points = m->get_vertex_property<vec3>("v:point");
				for (auto v : m->vertices())
					b.add_point(points[v]);
				m->set_bounding_box(b);
				box.add_box(b);
				++count;
			}
		}
		if (count > 0) {
			camera_->setSceneBoundingBox(box.min(), box.max());
			camera_->showEntireScene();
			update();
			return true;
		}
		return false;
	}


	Surface_mesh* Viewer::open_mesh(const std::string& file_name) {
		Surface_mesh* mesh = new Surface_mesh;
		if (!mesh->read(file_name) || mesh->n_faces() == 0) {
			std::cerr << "loading file failed" << std::endl;
			delete mesh;
			return nullptr;
		}
		mesh->set_name(file_name);
		models_.push_back(mesh);
		++model_idx_;

		std::cout << "file loaded" << std::endl
			<< "\tnum faces:    " << mesh->n_faces() << std::endl
			<< "\tnum vertices: " << mesh->n_vertices() << std::endl
			<< "\tnum edges:    " << mesh->n_edges() << std::endl;

		// create face drawable

		std::vector<unsigned int> indices;
		for (auto f : mesh->faces()) {
			int test_n = 0;
			for (auto v : mesh->vertices(f)) {
				indices.push_back(v.idx());
				++test_n;
			}
			if (test_n != 3)
				std::cerr << "now only triangles can be rendered" << std::endl;
		}

		auto points = mesh->get_vertex_property<vec3>("v:point");
		FacesDrawable* faces = mesh->add_face_drawable("surface");
		faces->update_vertex_buffer(points.vector());
		faces->update_index_buffer(indices);
		return mesh;
	}


	bool Viewer::save() const {
		std::vector< std::pair<std::string, std::string> > filetypes = {
			{"obj", "Wavefront Mesh"},
			{"off", "Object File Format"},
			{"ply", "PLY Mesh or Point Cloud"}
		};
		const std::string& file_name = easy3d::file_dialog(filetypes, true);
		if (file_name.empty())
			return false;
		return false;
	}


	void Viewer::draw_corner_axes() {
		if (!lines_program_)
			return;

		if (!axes_) {
			float len = 0.7f;
			std::vector<vec3> points = { vec3(0,0,0), vec3(len,0,0), vec3(0,0,0), vec3(0,len,0), vec3(0,0,0), vec3(0,0,len) };
			std::vector<vec3> colors = { vec3(1,0,0), vec3(1,0,0), vec3(0,1,0), vec3(0,1,0), vec3(0,0,1), vec3(0,0,1) };
			axes_ = new LinesDrawable("corner_axes");
			axes_->update_vertex_buffer(points);
			axes_->update_color_buffer(colors);
		}
		// The viewport and the scissor are changed to fit the lower left corner.
		int scissor[4];
		glGetIntegerv(GL_SCISSOR_BOX, scissor);	mpl_debug_gl_error;

		static int corner_frame_size = 150;
		glViewport(0, 0, corner_frame_size, corner_frame_size);
		glScissor(0, 0, corner_frame_size, corner_frame_size);	mpl_debug_gl_error;

		const mat4& proj = easy3d::ortho(-1, 1, -1, 1, -1, 1);
		const mat4& view = camera_->orientation().inverse().matrix();
		const mat4& MVP = proj * view;
		lines_program_->bind();
		lines_program_->set_uniform("MVP", MVP);
		lines_program_->set_uniform("per_vertex_color", true);
		lines_program_->set_uniform("default_color", vec3(0.4f, 0.8f, 0.8f));		mpl_debug_gl_error;
		axes_->draw(false);					mpl_debug_gl_error;
		lines_program_->unbind();

		glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
		glViewport(0, 0, width_, height_);
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
	}


	void Viewer::draw() {
		if (models_.empty())
			return;

		// Makes the depth coordinates of the filled primitives smaller, so that
		// displaying the mesh and the surface does not cause Z-fighting.
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.5f, -0.0001f);

		surface_program_->bind();	mpl_debug_gl_error;
		const mat4& MVP = camera_->modelViewProjectionMatrix();
		surface_program_->set_uniform("MVP", MVP);		mpl_debug_gl_error;
		// light is defined in VC
		vec4 eyeLightPos(0.27f, 0.27f, 0.92f, 0);
		const mat4& MV = camera_->modelViewMatrix();
		const vec4& wLightPos = inverse(MV) * eyeLightPos;
		surface_program_->set_uniform("wLightPos", wLightPos);		mpl_debug_gl_error;
		// NOTE: camera position is defined in world coordinate system.
		const vec3& wCamPos = camera_->position();
		// it can also be computed as follows:
		//const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
		surface_program_->set_uniform("wCamPos", wCamPos);		mpl_debug_gl_error;
		surface_program_->set_uniform("ambient", vec4(0.05f, 0.05f, 0.05f, 1.0f));		mpl_debug_gl_error;
		surface_program_->set_uniform("specular", vec4(0.4f, 0.4f, 0.4f, 1.0f));		mpl_debug_gl_error;
		surface_program_->set_uniform("shininess", 64.0f);		mpl_debug_gl_error;
		surface_program_->set_uniform("per_vertex_color", false);		
		for (std::size_t idx = 0; idx < models_.size(); ++idx) {
			Model* m = models_[idx];
			if (!m->is_visible()) 
				continue;
			surface_program_->set_uniform("default_color", 
				idx == model_idx_ ? vec3(0.4f, 0.8f, 0.8f) : vec3(0.8f, 0.8f, 0.8f));		mpl_debug_gl_error;
			for (auto d : m->face_drawables()) {
				if (d->is_visible())
					d->draw(false);
			}
		}
		surface_program_->unbind();	mpl_debug_gl_error;
		glDisable(GL_POLYGON_OFFSET_FILL);

		lines_program_->bind();
		lines_program_->set_uniform("MVP", MVP);
		lines_program_->set_uniform("per_vertex_color", false);
		lines_program_->set_uniform("default_color", vec3(0.0f, 0.0f, 0.0f));		mpl_debug_gl_error;
		for (auto m : models_) {
			if (!m->is_visible())
				continue;
			for (auto d : m->line_drawables()) {
				if (d->is_visible())
					d->draw(false);
			}
		}
		lines_program_->unbind();
	}


}
