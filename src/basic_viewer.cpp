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

#include "basic_viewer.h"

#include <thread>
#include <chrono>
#include <iostream>


#if !defined(_WIN32)
#  include <unistd.h>
#  include <sys/wait.h>
#endif

#include <GL/glew.h>    // Initialize with glewInit()
#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions

#include "camera.h"
#include "manipulatedCameraFrame.h"
#include "file_dialog.h"


// Internal global variables used for glfw event handling
static BasicViewer * __viewer;


BasicViewer::BasicViewer(
	const std::string& title /* = "Easy3D BasicViewer" */,
	int samples /* = 4 */,
	int gl_major /* = 3 */,
	int gl_minor /* = 2 */,
	bool full_screen /* = false */,
	bool resizable /* = true */,
	int depth_bits /* = 24 */,
	int stencil_bits /* = 8 */
)
	: title_(title)
	, full_screen_(false)
	, visible_(false)
	, process_events_(true)
	, samples_(0)
	, surface_(nullptr)
	, pointcloud_(nullptr)
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
        window_ = glfwCreateWindow(1280, 960, title.c_str(), nullptr, nullptr);
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
	printf("OpenGL Version %d.%d loaded\n", gl_major, gl_minor);
	int major = glfwGetWindowAttrib(window_, GLFW_CONTEXT_VERSION_MAJOR);
	int minor = glfwGetWindowAttrib(window_, GLFW_CONTEXT_VERSION_MINOR);
	int rev = glfwGetWindowAttrib(window_, GLFW_CONTEXT_REVISION);
	printf("OpenGL version received: %d.%d.%d\n", major, minor, rev);
	printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
	printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif

	if (strstr((const char *)glGetString(GL_VENDOR), "Intel") != nullptr) {
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

	visible_ = glfwGetWindowAttrib(window_, GLFW_VISIBLE) != 0;
	background_color_[0] = background_color_[1] = background_color_[2] = 0.3f;
	mouse_x_ = mouse_y_ = 0;
	button_ = -1;
	modifiers_ = 0;
	drag_active_ = false;
	process_events_ = true;

	camera_ = new Camera;
	camera_->setSceneRadius(1.0f);
	camera_->setSceneCenter(vec3(0, 0, 0));
	int w, h;
	glfwGetWindowSize(window_, &w, &h);
	camera_->setScreenWidthAndHeight(w, h);

	__viewer = this;

#if defined(__APPLE__)
    /* Poll for events once before starting a potentially lengthy loading process.*/
	glfwPollEvents();
#endif
}


void BasicViewer::setup_callbacks() {
	glfwSetCursorPosCallback(window_, [](GLFWwindow *win, double x, double y)
	{
		if (!__viewer->process_events_)
			return;

		int w, h;
		glfwGetWindowSize(win, &w, &h);
		if (x >= 0 && x <= w && y >= 0 && y <= h)
			__viewer->callback_event_cursor_pos(x, y);
        else if (__viewer->drag_active_) {
			// Restrict the cursor to be within the client area during dragging
			if (x < 0) x = 0;	if (x > w) x = w;
			if (y < 0) y = 0;	if (y > h) y = h;
			glfwSetCursorPos(win, x, y);
		}
	});

	glfwSetMouseButtonCallback(window_, [](GLFWwindow *win, int button, int action, int modifiers) {
		if (!__viewer->process_events_)
			return;
		__viewer->callback_event_mouse_button(button, action, modifiers);
	});

	glfwSetKeyCallback(window_, [](GLFWwindow *win, int key, int scancode, int action, int mods) {
		if (!__viewer->process_events_)
			return;
		(void)scancode;
		__viewer->callback_event_keyboard(key, action, mods);
	});

	glfwSetCharCallback(window_, [](GLFWwindow *win, unsigned int codepoint) {
		if (!__viewer->process_events_)
			return;
		__viewer->callback_event_character(codepoint);
	});

	glfwSetDropCallback(window_, [](GLFWwindow *win, int count, const char **filenames) {
		if (!__viewer->process_events_)
			return;
		__viewer->callback_event_drop(count, filenames);
	});

	glfwSetScrollCallback(window_, [](GLFWwindow *win, double dx, double dy) {
		if (!__viewer->process_events_)
			return;
		__viewer->callback_event_scroll(dx, dy);
	});

	/* React to framebuffer size events -- includes window size events
	   and also catches things like dragging a window from a Retina-capable
	   screen to a normal screen on Mac OS X */
	glfwSetFramebufferSizeCallback(window_, [](GLFWwindow *win, int width, int height) {
		if (!__viewer->process_events_)
			return;
         __viewer->callback_event_resize(width, height);
	});

	// notify when the screen has lost focus (e.g. application switch)
	glfwSetWindowFocusCallback(window_, [](GLFWwindow *win, int focused) {
		// focused: 0 when false, 1 when true
		__viewer->focus_event(focused != 0);
	});

	glfwSetWindowCloseCallback(window_, [](GLFWwindow *win) {
		glfwSetWindowShouldClose(win, true);
	});
}


BasicViewer::~BasicViewer() {
	destroy();
}


void BasicViewer::destroy() {
	// viewer may have already been destroyed by the user
	if (!window_)
		return;

	if (camera_)		delete camera_;
	//if (surface_)		delete surface_;
	//if (pointcloud_)	delete pointcloud_;

	cleanup();

	glfwDestroyWindow(window_);
	window_ = nullptr;
	glfwTerminate();
}


void BasicViewer::set_title(const std::string &title) {
	if (title != title_) {
		glfwSetWindowTitle(window_, title.c_str());
		title_ = title;
	}
}


void BasicViewer::set_visible(bool visible) {
	if (visible_ != visible) {
		visible_ = visible;
		if (visible)
			glfwShowWindow(window_);
		else
			glfwHideWindow(window_);
	}
}


// Returned values are(0, 0, screen_width, screen_height), so that the origin is
// located in the lower left corner of the window (OpenGL style coordinate system).
void BasicViewer::get_viewport(int viewport[4]) const {
	glfwMakeContextCurrent(window_);
	glGetIntegerv(GL_VIEWPORT, viewport);
}


void BasicViewer::set_background_color(float r, float g, float b) {
	background_color_[0] = r;
	background_color_[1] = g;
	background_color_[2] = b;
}


void BasicViewer::resize(int w, int h) {
    glfwSetWindowSize(window_, w, h);
}


void BasicViewer::repaint() {
	glfwPostEmptyEvent();
}


bool BasicViewer::mouse_press_event(int x, int y, int button, int modifiers) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && modifiers == 0)
		camera_->frame()->mousePressEvent(x, y, button, modifiers, camera_);
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && modifiers == 0) {

	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && modifiers == GLFW_MOD_SHIFT) {
		if (!camera_->setPivotPointFromPixel(x, y)) {
			camera_->setPivotPoint(camera_->sceneCenter());
			std::cout << "set pivot point as scene center" << std::endl;
		}
		else {
			std::cout << "set pivot point as scene center" << std::endl;
		}
	}

	//repaint();
	return false;
}


bool BasicViewer::mouse_release_event(int x, int y, int button, int modifiers) {
	std::string msg;
	if (button == GLFW_MOUSE_BUTTON_LEFT)
		msg += "Left button released";
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
		msg += "Right button released";

	if (modifiers == GLFW_MOD_CONTROL)
		msg += " + CTRL modifier";
	else if (modifiers == GLFW_MOD_SHIFT)
		msg += " + SHIFT modifier";

	std::cout << title_ + ": " + msg + " (" << x << ", " << y << ")" << std::endl;

	button_ = -1;

	//repaint();
	return false;
}


bool BasicViewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
	if (modifiers == 0) {
		if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT) {
			camera_->frame()->mouseMoveEvent(x, y, dx, dy, button, modifiers, camera_);
		}
	}

	//repaint();
	return false;
}


bool BasicViewer::mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) {
	// highlight geometry primitives here

	//std::cout << title_ + ": free move (" << x << ", " << y << "), delta (" << dx << ", " << dy << ")" << std::endl;

	//repaint();
	return false;
}


bool BasicViewer::mouse_scroll_event(int x, int y, int dx, int dy) {
	try {
		camera_->frame()->wheelEvent(x, y, dx, dy, camera_);
	}
	catch (const std::exception &e) {
		std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
	}

	//repaint();
	return false;
}


bool BasicViewer::key_press_event(int key, int modifiers) {
	try {
		if (key == GLFW_KEY_C && modifiers == 0) {
			camera_->centerScene();
			std::cout << "center scene" << std::endl;
		}
		else if (key == GLFW_KEY_F && modifiers == 0) {
			camera_->showEntireScene();
			std::cout << "show entire scene" << std::endl;
		}
		else if (key == GLFW_KEY_LEFT) {
			std::cout << title_ + ": Key_LEFT pressed" << std::endl;
		}
		else if (key == GLFW_KEY_RIGHT) {
			std::cout << title_ + ": Key_RIGHT pressed" << std::endl;
		}
		else if (key == GLFW_KEY_UP) {
			std::cout << title_ + ": Key_UP pressed" << std::endl;
		}
		else if (key == GLFW_KEY_DOWN) {
			std::cout << title_ + ": Key_DOWN pressed" << std::endl;
		}
		else if (key == GLFW_KEY_M) {
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
	}
	catch (const std::exception &e) {
		std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
	}

	//repaint();
	return false;
}


bool BasicViewer::key_release_event(int key, int modifiers) {
	try {
	}
	catch (const std::exception &e) {
		std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
	}

	//repaint();
	return false;
}


bool BasicViewer::char_input_event(unsigned int codepoint) {
	switch (codepoint) {
	case '-':
		std::cout << title_ + ": " + "\'-\' pressed" << std::endl;
		break;
	case '=':
		std::cout << title_ + ": " + "\'=\' pressed" << std::endl;
		break;
	case '+':
		std::cout << title_ + ": " + "\'+\' pressed" << std::endl;
		break;
	case 'c':
		std::cout << title_ + ": " + "\'c\' pressed" << std::endl;
		break;
	case 'C':
		std::cout << title_ + ": " + "\'C\' pressed" << std::endl;
		break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		std::cout << title_ + ": " + "\'" << char(codepoint) << "\' pressed" << std::endl;
		break;
	default:
		return false;
	}
	//repaint();
	return false;
}


void BasicViewer::draw_all() {
	glfwMakeContextCurrent(window_);
	glClearColor(background_color_[0], background_color_[1], background_color_[2], 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// ----------- pre-draw -------------------

	if (pre_draw())
		return;

	// ----------------draw -------------------

	draw();

	// -----------post-draw -------------------

	if (post_draw())
		return;


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


bool BasicViewer::callback_event_cursor_pos(double x, double y) {
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


bool BasicViewer::callback_event_mouse_button(int button, int action, int modifiers) {
	try {
		if (action == GLFW_PRESS) {
			drag_active_ = true;
			button_ = button;
			modifiers_ = modifiers;
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


bool BasicViewer::callback_event_keyboard(int key, int action, int modifiers) {
	try {
		if (action == GLFW_PRESS) {
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


bool BasicViewer::callback_event_character(unsigned int codepoint) {
	try {
		return char_input_event(codepoint);
	}
	catch (const std::exception &e) {
		std::cerr << "Caught exception in event handler: " << e.what()
			<< std::endl;
		return false;
	}
}


bool BasicViewer::callback_event_drop(int count, const char **filenames) {
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


bool BasicViewer::callback_event_scroll(double dx, double dy) {
	try {
		return mouse_scroll_event(mouse_x_, mouse_y_, static_cast<int>(dx), static_cast<int>(dy));
	}
	catch (const std::exception &e) {
		std::cerr << "Caught exception in event handler: " << e.what()
			<< std::endl;
		return false;
	}
}

void BasicViewer::callback_event_resize(int w, int h) {
	if (w == 0 && h == 0)
        return;

	try {
		glViewport(0, 0, w, h);
		camera_->setScreenWidthAndHeight(w, h);
		post_resize(w, h);
	}
	catch (const std::exception &e) {
		std::cerr << "Caught exception in event handler: " << e.what()
			<< std::endl;
	}
}


bool BasicViewer::focus_event(bool focused) {
	/* To be overridden */
	if (focused)
		std::cout << title_ << ": focused" << std::endl;

	return false;
}


inline double get_seconds() {
	return std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count();
}


void BasicViewer::run() {
	init();

	set_visible(true);

	// TODO: make it member variable
    bool is_animating = true;

	try {
		// Rendering loop
		const int num_extra_frames = 5;
		const double animation_max_fps = 30;
		int frame_counter = 0;

		while (!glfwWindowShouldClose(window_)) {
			if (!is_visible())
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

	destroy();
}


void BasicViewer::open_file() {
	std::vector< std::pair<std::string, std::string> > filetypes = {
		{"obj", "Wavefront Mesh"},
		{"ply", "ply Mesh"},
	};
	const std::string& file_name = Easy3D::file_dialog(filetypes, false);
}


void BasicViewer::save_file() {

}
