/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
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

#ifndef _EASY3D_BASIC_VIEWER_H_
#define _EASY3D_BASIC_VIEWER_H_

#include <map>
#include <string>
#include <vector>

#include <easy3d/types.h>

/*  Easy3D is really easy to use. That's why it has such a name.
*
*   Just create an instance of Viewer, or derive your viewer 
*   from it by re-implementing its draw() function (and other 
*   functions if necessary), and run your viewer:
*		MyViewer viewer("Example");
*       viewer.run();
*/


struct GLFWwindow;

namespace easy3d {

	class Camera;
	class Model;
	class Point_cloud;
	class Surface_mesh;
	class ShaderProgram;
	class LinesDrawable;

	class Viewer
	{
	public:
		Viewer(
			const std::string& title = "Easy3dViewer",
			int samples = 4,
			int gl_major = 3,
			int gl_minor = 2,
			bool full_screen = false,
			bool resizable = true,
			int depth_bits = 24,
			int stencil_bits = 8
		);

		virtual ~Viewer();

		// Enter the application main loop.
		void run();

        std::string usage() const;

		//-----------------------------------------------------

		void set_title(const std::string &title);
		const std::string& title() const { return title_; }

		int width() const { return width_; }
		int height() const { return height_; }

		const vec3& background_color() const { return background_color_; }
		void set_background_color(const vec3& c) { background_color_ = c; }

		/// Return the actual samples of the viewer
		int	 samples() const { return samples_; }

		// Returns the coordinates of the 3D point located at pixel (x,y) on screen.
		// x, y: screen point expressed in pixel units with an origin in the upper left corner.
		// found: indicates whether a point was found or not.
		// NOTE: This method assumes that a GL context is available, and that its
		//		 content was drawn using the Camera (i.e. using its projection and modelview
		//		 matrices). This method hence cannot be used for offscreen Camera computations.
		//		 Use cameraCoordinatesOf() and worldCoordinatesOf() to perform similar 
		//		 operations in that case.
		//       The precision of the z-Buffer highly depends on how the zNear() and zFar() 
		//       values are fitted to your scene. Loose boundaries will result in imprecision 
		//		 along the viewing direction.
		vec3 point_under_pixel(int x, int y, bool &found) const;

		/// Update the rendering
		void update() const;

		// explicitly set window size
		void resize(int w, int h);

		// Open/Save models specified by a file dialog.
		bool open();
		bool save() const;

		// The 'model' can be a point cloud or a mesh
		void add_model(Model* model);

	protected:

		// OpenGL resources (e.g., shaders, textures, VAOs) must be created when 
		// there exists a valid rendering context. It is (usually) a bad idea to do 
		// this in a constructor because the OpenGL context may not have been 
		// created yet or the visible one is not *current*. This init() function is 
		// to ensure you have a valid rendering context. It will be called before 
		// the draw procedure. See also cleanup().
		// NOTE: Don't forget to call Viewer::init() at the beginning of your 
		//		 inherited function.
		virtual void init();

		// OpenGL resources (e.g., shaders, textures, VAOs) must destroyed when 
		// there exists a valid rendering context. It is (usually) a bad idea to 
		// clean up OpenGL in a destructor because the OpenGL context may not exist
		// (e.g., destroyed already) or the visible one is not *current*. This
		// cleanup() function is to ensure you have a valid rendering context. 
		// See also init().
		// NOTE: Don't forget to call Viewer::cleanup() at the end of your 
		//		 inherited function.
		virtual void cleanup();

		// This function will be called before the main draw procedure.
		virtual void pre_draw();

		/// Put your rendering calls here
		virtual void draw();

		// This function will be called after the main draw procedure.
		virtual void post_draw();

		// External resize due to user interaction.
		// This function will be called after the window size being changed.
		virtual void post_resize(int w, int h) { /* To be overridden */ }

		/// Mouse button press event handler
		virtual bool mouse_press_event(int x, int y, int button, int modifiers);
		/// Mouse button release event handler
		virtual bool mouse_release_event(int x, int y, int button, int modifiers);
		/// Mouse drag (i.e., a mouse button was pressed) event handler
		virtual bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers);
		/// Mouse free move (i.e., no mouse button was pressed) event handler
		virtual bool mouse_free_move_event(int x, int y, int dx, int dy, int modifiers);
		/// Mouse scroll event handler
		virtual bool mouse_scroll_event(int x, int y, int dx, int dy);

		/// Text input event handler: codepoint is native endian UTF-32 format
		/// NOTE: This one reveals the actual character being sent (not just the physical key)
		virtual bool char_input_event(unsigned int codepoint);
		/// Keyboard event handler. 
		/// NOTE: This function does not reveal the actual character.
		virtual bool key_press_event(int key, int modifiers);
		virtual bool key_release_event(int key, int modifiers);

		/// Handle a file drop event
		virtual bool drop_event(const std::vector<std::string> & filenames);

		/// Handle a focus change event
		virtual bool focus_event(bool focused);

	protected:
		// Open a file with file name given
		// NOTE: model will not be added to the viewer
		Model* open(const std::string& file_name);

        void draw_corner_axes();

        void setup_callbacks();

		/* Event handlers. Client code should not touch these */
		virtual bool callback_event_cursor_pos(double x, double y);
		virtual bool callback_event_mouse_button(int button, int action, int modifiers);
		virtual bool callback_event_keyboard(int key, int action, int mods);
		virtual bool callback_event_character(unsigned int codepoint);
		virtual bool callback_event_drop(int count, const char **filenames);
		virtual bool callback_event_scroll(double dx, double dy);
		virtual void callback_event_resize(int w, int h);

	protected:
		GLFWwindow*	window_;
        double      highdpi_;
		std::string	title_;
		Camera*		camera_;

		int		samples_;	// the actual samples available

		bool	full_screen_;
		int		width_;
		int		height_;
		vec3	background_color_;

		// enable/disable event processing
		bool	process_events_;

		// mouse
		int		button_;		// for mouse drag
		int		modifiers_;		// for mouse drag
		bool	drag_active_;	// for mouse drag
		int		mouse_x_;		// for mouse button and scroll events
		int		mouse_y_;		
		int     mouse_pressed_x_;	// last pressed position
		int     mouse_pressed_y_;
		int     pressed_key_;

		//----------------- viewer data -------------------

		// corner axes
		bool	show_corner_axes_;
		LinesDrawable* axes_;

		ShaderProgram* points_program_;
		ShaderProgram* lines_program_;
		ShaderProgram* surface_program_;

		std::vector<Model*> models_;
		int model_idx_;
	};

}


#endif	// _EASY3D_BASIC_VIEWER_H_
