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

#ifndef EASY3D_VIEWER_H
#define EASY3D_VIEWER_H


#include <string>
#include <vector>

#include <easy3d/core/types.h>

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
    class OpenGLTimer;
    class TrianglesDrawable;

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

        // explicitly set window size
        void resize(int w, int h);

        int width() const;
        int height() const;

        // Return the actual samples of the viewer
        int	 samples() const { return samples_; }

        // Scaling factor for high DPI devices
        double dpi_scaling() const { return dpi_scaling_; }

        // The background color of the viewer
        const easy3d::vec4& background_color() const { return background_color_; }
        void set_background_color(const easy3d::vec4& c) { background_color_ = c; }

        // the camera
        Camera* camera() { return camera_; }
        const Camera* camera() const { return camera_; }

        // -------------------- fileIO, model management ----------------------

        // returns all the models managed by this viewer
        const std::vector<Model*>& models() const { return models_; }

        // returns the active model
        Model* current_model() const;

        // Open/Save models specified by using a file dialog.
        bool open();
        bool save() const;

        // Open a file (given the file name) and add the model to the viewer
        // for visualization (the viewer will be incharge of its memory menagement).
        // By default, this function also It will also creates necessary drawables
        // for visualizing the model. Set create_default_drawables to false if you
        // want to create a customized drawable for a sepcific rendering purpose.
        Model* open(const std::string& file_name, bool create_default_drawables = true, bool smooth_shading = true);

        // Add a model to the viewer to be visualized (the viewer will be incharge
        // of its memory menagement).
        // By default, this function also It will also creates necessary drawables
        // for visualizing the model. Set create_default_drawables to false if you
        // want to create a customized drawable for a sepcific rendering purpose.
        void add_model(Model* model, bool create_default_drawables = true, bool smooth_shading = true);

        // delete the model from the viewer. The model will also be destroyed.
        void delete_model(Model* model);

        // ----------------------------- UI -----------------------------------

        // moves the camera so that the 'model' is centered on the screen.
        // if 'model' is NULL, it centers the entire scene (all models).
        virtual void fit_screen(const easy3d::Model* model = nullptr);

        // take a snapshot of the screen and save the snapshot into a file (the file name
        // specified by using a file dialog)
        bool snapshot(bool bk_white = true) const;

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

        // rendering. Users can put their additional rendering function here by reimplementing it.
        virtual void draw() const;

        // Update the rendering
        static void update();

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

		// This function will be called after the main draw procedure.
		virtual void post_draw();

		// External resize due to user interaction.
		// This function will be called after the window size being changed.
        virtual void post_resize(int w, int h) { (void)w, (void)h; }

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

        // Create default drawables for rendering
        //  - for point clouds, it creates a PointsDrawable
        //      - per point color will be enabled if vertex property 'v:color' exists
        //      - nomals (stored in vertex property 'v:normal') will used for rendering if exists.
        //  - for mesh surfaces, it creates a TrianglesDrawable
        //      - per vertex color will be enabled if vertex property 'v:color' exists
        // TODO: move this function to Renderer module; enable per face color for surface meshes.
        void create_drawables(Model* m, bool smooth_shading = true);

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
        double      dpi_scaling_;

		std::string	title_;
		Camera*		camera_;

        int		samples_;	// the actual samples

		bool	full_screen_;
        vec4	background_color_;

		// enable/disable event processing
		bool	process_events_;

        OpenGLTimer* gpu_timer_;
        double gpu_time_;

        // key
        int     key_;

		// mouse
		int		button_;		// for mouse drag
		int		modifiers_;		// for mouse drag
		bool	drag_active_;	// for mouse drag
		int		mouse_x_;		// for mouse button and scroll events
		int		mouse_y_;		
		int     mouse_pressed_x_;	// last pressed position
		int     mouse_pressed_y_;

		//----------------- viewer data -------------------

		// corner axes
		bool	show_corner_axes_;
        TrianglesDrawable* axes_;

        bool	show_pivot_point_;
        vec2    pivot_point_;

		std::vector<Model*> models_;
		int model_idx_;
	};

}


#endif	// EASY3D_VIEWER_H
