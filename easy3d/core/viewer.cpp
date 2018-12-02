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

#include "camera.h"
#include "manipulated_camera_frame.h"
#include <easy3d/model/surface_mesh.h>
#include <easy3d/model/point_cloud.h>
#include "drawable.h"
#include "shader_program.h"
#include "shader_code.h"
#include "opengl_error.h"
#include "file_dialog.h"


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
		, axis_ (nullptr)
		, axis_program_(nullptr)
		, surface_program_(nullptr)
		, width_(1280)	// default width
		, height_(960)	// default height
		, pressed_key_(GLFW_KEY_UNKNOWN)
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
			else {
				drag_active_ = false;
				return mouse_release_event(mouse_x_, mouse_y_, button, modifiers);
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

		if (axis_) {
			delete axis_;
			axis_ = nullptr;
		}

		if (axis_program_) {
			delete axis_program_;
			axis_program_ = nullptr;
		}

		for (auto model : surface_drawables_) {
			delete model.second;
			delete model.first;
		}
		surface_drawables_.clear();

		if (surface_program_) {
			delete surface_program_;
			surface_program_ = nullptr;
		}

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


	// Returned values are(0, 0, screen_width, screen_height), so that the origin is
	// located in the lower left corner of the window (OpenGL style coordinate system).
	void Viewer::get_viewport(int viewport[4]) const {
		glfwMakeContextCurrent(window_);
		glGetIntegerv(GL_VIEWPORT, viewport);
	}


	void Viewer::set_background_color(float r, float g, float b) {
		background_color_[0] = r;
		background_color_[1] = g;
		background_color_[2] = b;
	}


	void Viewer::resize(int w, int h) {
		glfwSetWindowSize(window_, w, h);
	}


	void Viewer::update() const {
		glfwPostEmptyEvent();

		//// Liangliang: force process all event
		//glfwPollEvents();
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
		bool found = false;
		const vec3& p = point_under_pixel(x, y, found);
		if (found)
			std::cout << "point under mouse: " << p << std::endl;
		// highlight geometry primitives here
		return false;
	}


	bool Viewer::mouse_scroll_event(int x, int y, int dx, int dy) {
		camera_->frame()->action_zoom(dy, camera_);
		return false;
	}


	bool Viewer::key_press_event(int key, int modifiers) {
		if (key == GLFW_KEY_C && modifiers == 0) {
			camera_->centerScene();
		}
		else if (key == GLFW_KEY_F && modifiers == 0) {
			camera_->showEntireScene();
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
			std::cout << usage_ << std::endl;
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


	void Viewer::pre_draw() {
		const mat4& MVP = camera_->modelViewProjectionMatrix();
		axis_program_->bind();
		axis_program_->set_uniform("MVP", MVP);
		axis_program_->set_uniform("color", vec3(1.f, 0.f, 0.f));		mpl_debug_gl_error;
		axis_->draw(false);					mpl_debug_gl_error;
		axis_program_->unbind();
	}


	void Viewer::draw_all() {
		glfwMakeContextCurrent(window_);
		glClearColor(background_color_[0], background_color_[1], background_color_[2], 1.0f);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// --------------------------------------

		pre_draw();		mpl_debug_gl_error;
		draw();			mpl_debug_gl_error;
		post_draw();	mpl_debug_gl_error;

		// --------------------------------------

		// Liangliang: Use this to implement a timer
		//double elapsed = glfwGetTime() - last_interaction_;
		//if (elapsed > 0.5f) {
		//	/* Draw tooltips */
		//	const Widget *widget = findWidget(mMousePos);
		//	if (widget && !widget->tooltip().empty()) {
		//		int tooltipWidth = 150;
		//		// ...
		//	}
		//}
	}


	void Viewer::init() {
		std::vector<vec3> points = {vec3(0,0,0), vec3(1,0,0), vec3(0,1,0), vec3(0,0,1)};
		std::vector<unsigned int> indices = { 0, 1, 0, 2, 0, 3 };
		axis_ = new LinesDrawable;
		axis_->update_vertex_buffer(points);
		axis_->update_index_buffer(indices);
		axis_program_ = new ShaderProgram;
		axis_program_->load_shader_from_code(ShaderProgram::VERTEX,
			"#version 330\n"
			"uniform mat4 MVP;\n"
			"in vec3 vtx_position;\n"
			"void main() {\n"
			"    gl_Position = MVP * vec4(vtx_position, 1.0);\n"
			"}"
		);
		axis_program_->load_shader_from_code(ShaderProgram::FRAGMENT,
			"#version 330\n"
			"uniform vec3 color;\n"
			"out vec4 outputColor;\n"
			"void main() {\n"
			"    outputColor = vec4(color, 1.0);\n"
			"}"
		);
		axis_program_->set_attrib_name(ShaderProgram::POSITION, "vtx_position");
		axis_program_->link_program();

		camera_ = new Camera;
		camera_->setSceneRadius(1.0f);
		camera_->setSceneCenter(vec3(0, 0, 0));
		int w, h;
		glfwGetFramebufferSize(window_, &w, &h);
		camera_->setScreenWidthAndHeight(w, h);

		// seems depth test is disabled by default
		glEnable(GL_DEPTH_TEST);
		glfwShowWindow(window_);

		usage_ = std::string(R"(Easy3D viewer usage:
  F1:              Help
  Ctrl + O:        Open file
  Ctrl + S:        Save file
  Left:            Rotate scene
  Right:           Translate scene
  Alt + Left:      Rotate scene (screen based)
  Alt + Right:     Translate scene (screen based)
  Middle/Wheel:    Zoom out/in
  Ctrl + '-'/'+':  Zoom out/in
  F:               Fit screen      
  C:               Center scene
  Shift + Right:   Set/unset pivot point
  P:               Toggle perspective/orthographic projection)"
			// W:               Toggle wireframe
			//   <,>     Toggle between models
			//   ;       Toggle vertex labels
			//   :       Toggle face labels)"			
		);
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
		bool is_animating = true;

		try {
			// Rendering loop
			const int num_extra_frames = 5;
			const double animation_max_fps = 30;
			int frame_counter = 0;

			while (!glfwWindowShouldClose(window_)) {
				if (!glfwGetWindowAttrib(window_, GLFW_VISIBLE)) // not visible
					continue;

				double tic = get_seconds();

				draw_all();
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


	bool Viewer::open() {
		std::vector< std::pair<std::string, std::string> > filetypes = {
			{"obj", "Wavefront Mesh"},
			{"off", "Object File Format"},
			{"ply", "PLY mesh or point cloud"},
			{"*", "All Files"}
		};
		const std::string& file_name = easy3d::file_dialog(filetypes, false);
		if (file_name.empty())
			return false;

		return open_mesh(file_name);
	}


	bool Viewer::open_mesh(const std::string& file_name) {
		Surface_mesh* mesh = new Surface_mesh;
		if (mesh->read(file_name)) {
			if (mesh->n_faces() > 0) {
				std::cout << "file loaded" << std::endl
					<< "\tnum faces:    " << mesh->n_faces() << std::endl
					<< "\tnum vertices: " << mesh->n_vertices() << std::endl
					<< "\tnum edges:    " << mesh->n_edges() << std::endl;

				// create face drawable
				FacesDrawable* faces = new FacesDrawable;
				surface_drawables_[mesh] = faces;

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
				faces->update_vertex_buffer(points.vector());
				faces->update_index_buffer(indices);

				// create shader program
				if (!surface_program_) {
					const std::string& code_name = "surface_color";
					surface_program_ = new ShaderProgram(code_name);
#if 0 // using files
					// Liangliang: Weird relative path doesn't work!!!!!! I have been using this for many years.
					const std::string& vert_file = "../../resources/shaders/surface_color.vert";
					const std::string& frag_file = "../../resources/shaders/surface_color.frag";
					if (surface_program_->load_shader_from_file(ShaderProgram::VERTEX, vert_file) &&
						surface_program_->load_shader_from_file(ShaderProgram::FRAGMENT, frag_file))
#else
					if (surface_program_->load_shader_from_code(ShaderProgram::VERTEX, surface_color_vert) &&
						surface_program_->load_shader_from_code(ShaderProgram::FRAGMENT, surface_color_frag))
#endif
					{
						surface_program_->set_attrib_name(ShaderProgram::POSITION, "vtx_position");
						surface_program_->link_program();
					}
					else {
						std::cerr << "failed loading shader program" << std::endl;
						delete surface_program_;
						surface_program_ = nullptr;
					}
				}

				// fit screen
				Box3 box;
				for (auto v : mesh->vertices()) {
					box.add_point(points[v]);
				}
				camera_->setSceneBoundingBox(box.min(), box.max());
				camera_->showEntireScene();
				update();
				return true;
			}
		}

		std::cerr << "loading file failed" << std::endl;
		delete mesh;
		return false;
	}


	bool Viewer::save() const {
		std::vector< std::pair<std::string, std::string> > filetypes = {
			{"obj", "Wavefront Mesh"},
			{"ply", "ply Mesh or Point Cloud"},
		};
		const std::string& file_name = easy3d::file_dialog(filetypes, true);
		if (file_name.empty())
			return false;
		return false;
	}


	bool Viewer::drop_event(const std::vector<std::string> & filenames) {
		for (auto& name : filenames) {
			open_mesh(name);
		}
		return false;
	}


	void Viewer::draw() {
		if (!surface_program_)
			return;

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
		surface_program_->set_uniform("color", vec3(0.4f, 0.8f, 0.8f));		mpl_debug_gl_error;

		for (auto md : surface_drawables_)
			md.second->draw(false);

		surface_program_->unbind();	mpl_debug_gl_error;
	}


}
