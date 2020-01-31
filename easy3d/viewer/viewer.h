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

#ifndef EASY3D_VIEWER_H
#define EASY3D_VIEWER_H


#include <string>
#include <vector>

#include <easy3d/core/types.h>


struct GLFWwindow;

namespace easy3d {

	class Camera;
	class Model;
    class Drawable;
    class OpenGLTimer;
	class LinesDrawable;
    class TrianglesDrawable;

    /**
     * @brief The built-in Easy3D Viewer.
     *
     * @details Easy3D is really easy to use. That's why it has such a name.
     *
     *   To use the viewer, simply create an instance of Viewer, and call the run() method, i.e.,
     *   -------------------------------------------------------------------
     * 		MyViewer viewer("Example");
     *      viewer.run();
     *   -------------------------------------------------------------------
     *   The default Easy3D viewer can be easily extended by deriving your enhanced viewer from it
     *   by re-implementing some of its unctions (see the documentation of Viewer).
     */
	class Viewer
	{
	public:
        /**
         * @brief Creating the Viewer
         * @param title The window title of the viewer.
         * @param samples The number of samples for multisample antialiasing
         * @param gl_major/gl_minor The OpenGL version to request. The created context will be
         *        compatible with the requested version (if the context creation succeeded).
         * @param full_screen Do you want the viewer to be fullscreen?
         * @param resizable Whether the viewer will be resizable by the user.
         * @param depth_bits The desired bit depths of the depth component of the default
         *        framebuffer.
         * @param stencil_bits The desired bit depths of the stencil component of the default
         *        framebuffer.
         */
		Viewer(
			const std::string& title = "Easy3dViewer",
			int samples = 4,
            int gl_major = 3,   // must >= 3
            int gl_minor = 2,   // must >= 2
            bool full_screen = false,
			bool resizable = true,
			int depth_bits = 24,
			int stencil_bits = 8
		);

        /**
         * @brief The desctructor of the Viewer.
         */
		virtual ~Viewer();

        /**
         * Run the viewer.
         */
        void run();


        /** -------------------- The viewer properties ---------------------- */

        /**
         * @brief Set the window title of the viewer.
         * @param title The string of the title.
         */
		void set_title(const std::string &title);
        /**
         * @brief Query the window title of the viewer.
         * @return The string of the window title.
         */
		const std::string& title() const { return title_; }

        /**
         * @brief Set/Change the viewer size.
         * @param w/h The requested width/height (in pixels) of the viewer.
         */
        void resize(int w, int h);

        /**
         * @brief Query the size of the viewer.
         * @return The width/height of the viewer.
         */
        int width() const;
        int height() const;

        /**
         * @brief Query the actual samples of the viewer.
         * @note The requested sample may not be supported by the context or graphics driver. This
         *       method returns the actual samples that the viewer supports.
         */
        int	 samples() const { return samples_; }

        /**
         * @brief Query the scaling factor for high DPI devices (e.g., MackBook pro).
         * @return The high DPI scaling factor.
         */
        double dpi_scaling() const { return dpi_scaling_; }

        /**
         * @brief Set the background color of the viewer
         * @param color The background color.
         */
        void set_background_color(const easy3d::vec4& c) { background_color_ = c; }

        /**
         * @brief Query the background color of the viewer.
         * @return The background color of the viewer
         */
        const easy3d::vec4& background_color() const { return background_color_; }

        /**
         * @brief Returns the camera used by the viewer. See camera.h.
         */
        Camera* camera() { return camera_; }
        const Camera* camera() const { return camera_; }

        /** ------------------------------------ fileIO -------------------------------------- */

        /**
         * @brief Open a model (PointCloud/SurfaceMesh/Graph) from a file into the viewer. On
         *        success, the viewer will be in charge of the memory management of the model.
         * @details This method loads a model into the viewer. Internally, it will pop up a file
         *          dialog for the user to navigate to the file. After loading the model, the
         *          necessary drawables (e.g., "vertices" for point clouds, "faces" for surface
         *          meshes, and "edges" and "vertices" for graphs) will be created for visualization.
         * @return true on success and false otherwise.
         */
        virtual bool open();

        /**
         * @brief Save the active model (if exists) to a file.
         * @details This method saves the active model to a file. Internally, it will pop up a file
         *          dialog for specifying the file name.
         * @return true on success and false otherwise.
         */
        virtual bool save() const;

        /** ------------------------------- model management --------------------------------- */

        /**
         * @brief Add a model from a file to the viewer to be visualized. On success, the viewer
         *        will be in charge of the memory management of the model. The loaded model can be
         *        accessed by the 'current_model()' method.
         * @details This method loads a model into the viewer. It allows the user to control if
         *          default drawables will be created. The default drawables are
         *          - for point clouds, a PointsDrawable (with name "vertices");
         *          - for surface meshes, a TrianglesDrawable (with name "faces");
         *          - for graphs, a PointsDrawable (with name "vertices") and a LinesDrawable (with
         *            name "edges").
         *          These drawables are usually sufficient for basic rendering of the model. In case
         *          the default drawables don't meet the particular visualization purpose, a user
         *          can set 'create_default_drawables' to false and create the drawables later on.
         * @param file_name The string of the file name.
         * @param create_default_drawables If ture, the default drawables will be created. Users can
         *        set create_default_drawables to false if a customized drawable will be created for
         *        a particular rendering purpose.
         * @return The pointer to the model added to the viewer (nullptr if failed).
         * @related create_drawables(Model* model).
         */
        virtual Model* add_model(const std::string& file_name, bool create_default_drawables = true);

        /**
         * @brief Add an existing model to the viewer to be visualized. After a model being added
         *        to the viewer, the viewer will be incharge of its memory menagement.
         * @details This method adds a model into the viewer. It allows the user to control if
         *          default drawables will be created. The default drawables are
         *          - for point clouds, a PointsDrawable (with name "vertices");
         *          - for surface meshes, a TrianglesDrawable (with name "faces");
         *          - for graphes, a PointsDrawable (with name "vertices") and a LinesDrawable (with
         *            name "edges").
         *          These drawables are usually sufficient for basic rendering of the model. In case
         *          the default drawables don't meet the particular visualization purpose, a user
         *          can set 'create_default_drawables' to false and create the drawables later on.
         * @param model The pointer to the model.
         * @param create_default_drawables If ture, the default drawables will be created. Users can
         *        set create_default_drawables to false if a customized drawable will be created for
         *        a particular rendering purpose.
         * @return The pointer to the model added to the viewer (nullptr if failed).
         * @related add_model(const std::string&, bool).
         */
        virtual Model* add_model(Model* model, bool create_default_drawables = true);

        /**
         * @brief Delete a model. The memory of the model will be released and its existing drawables
         *        also be deleted.
         * @param model The pointer to the model.
         * @return True if the model has been deleted.
         */
        bool delete_model(Model* model);

        /**
         * @brief Query the models managed by this viewer.
         * @return The models managed by this viewer.
         */
        const std::vector<Model*>& models() const { return models_; }

        /**
         * @brief Query the active model.
         * @details The viewer can manage/visiulize/process multiple models. The default behavior
         *          of the Easy3D viewer is, when a command is triggerred (e.g., the Save menu was
         *          clicked), only the active mdoel is processed. This method is used to identify
         *          the active model.
         * @return The active model.
         */
        Model* current_model() const;

        /** ------------------------------------ drawable management ---------------------------------- */

        /**
         * @brief Add a drawable to the viewer to be visualized. After a drawable being added to the
         *        viewer, the viewer will be in charge of its memory management.
         * @details The use of drawables for visualization is quite flexible. Drawables are
         *          typically created for rendering 3D models (e.g., point clouds, meshes, graphs)
         *          and a 3D model is usually loaded from a file or generated by an algorithm. This
         *          method allows the user to visualize drawables without defining a 3D model.
         * @param drawable The pointer to the drawable.
         * @return True if the drawable has been added.
         */
	    bool add_drawable(Drawable* drawable);

        /**
         * Delete the drawable from the viewer. The related drawables will also be deleted.
         * @param drawable The pointer to the drawable.
         * @return True if the drawable has been deleted.
         */
	    bool delete_drawable(Drawable* drawable);

        /**
         * @brief Query the drawables managed by this viewer.
         * @return The drawables managed by this viewer.
         */
        const std::vector<Drawable*>& drawables() const { return drawables_; }

        /**
         * @brief Delete all visual contents of the viewer (all models and drawables).
         */
        void clear_scene();

        /** ----------------------------- UI ----------------------------------- */

        /**
         * @brief Moves the camera so that the entire scene or the active model is centered on the
         *        screen at a proper scale.
         * @param model The pointer to the model to be centered on the screen. If nullptr, the
         *        entire scene (i.e., all models) will be centered on the screen at a proper scale.
         */
        virtual void fit_screen(const easy3d::Model* model = nullptr);

        /**
         * @brief Take a snapshot of the screen and save it to a file.
         * @details This method takes a snapshot of the screen and saves the snapshot into a file
         *          Internally, it will pop up a file dialog for specifying the file name.
         * @return true on success and false otherwise.
         */
	    bool snapshot(bool bk_white = true) const;

        /**
         * @brief Query the XYZ coordinates of the surface point under the cursor.
         * @param x The cursor x-coordinate, relative to the left edge of the content area.
         * @param y The cursor y-coordinate, relative to the top edge of the content area.
         * @param found indicates whether the point was found or not.
         * @return The coordinates of the 3D point located at pixel (x,y) on screen. The returned point is valid only
         *         if found was returned true.
         * @attention The screen point (x, y) is expressed in the screen coordinate system with an origin in the upper
         *            left corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         * @note This method assumes that a GL context is available, and that its content was drawn using the Camera
         *       (i.e. using its projection and modelview matrices). This method hence cannot be used for offscreen
         *       Camera computations. Use cameraCoordinatesOf() and worldCoordinatesOf() to perform similar operations
         *       in that case. The precision of the method highly depends on the z-Buffer, i.e., how the zNear() and
         *       zFar() values are fitted to your scene. Loose boundaries will result in imprecision along the viewing
         *       direction.
         */
		vec3 point_under_pixel(int x, int y, bool &found) const;

        /**
         * @brief Update the display (i.e., repaint).
         * @details This method is used to update the display of the rendering. Client should call
         *          it when your data/view is changed.
         */
        void update() const;

        /**
         * @brief The usage information of the viewer. For the time being, it is the manual of the
         *        viewer. User can override the usage.
         */
	    virtual std::string usage() const;

	protected:
        /**
         * @brief Create default drawables for rendering
         * @details For a PointCloud, it creates a PointsDrawable. Per point color will be enabled
         *          if vertex property 'v:color' exists; nomals (stored in vertex property
         *          'v:normal') will be used for rendering if exists.
         *          For a MeshSurface, it creates a TrianglesDrawable; Per vertex color will be
         *          enabled if vertex property 'v:color' exists.
         *          For a Graph, it creates a PointsDrawable (phere imposters) for visualizing the
         *          vertices and a LinesDrawable (cylinder imposters) for visualizing the edges.
         * @todo TODO: move this function to Renderer module; enable per face color for surface meshes.
         */
        virtual void create_drawables(Model* model);

        // rendering. Users can put their additional rendering function here by reimplementing it.
        virtual void draw() const;

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
        GLFWwindow*	create_window(const std::string& title,
                                  int samples,
                                  int gl_major,   // must >= 3
                                  int gl_minor,   // must >= 2
                                  bool full_screen,
                                  bool resizable,
                                  int depth_bits = 24,
                                  int stencil_bits = 8);
        void setup_callbacks(GLFWwindow*);

		/* Event handlers. Client code should not touch these */
        virtual bool callback_event_cursor_pos(double x, double y);
        virtual bool callback_event_mouse_button(int button, int action, int modifiers);
        virtual bool callback_event_keyboard(int key, int action, int mods);
        virtual bool callback_event_character(unsigned int codepoint);
        virtual bool callback_event_drop(int count, const char **filenames);
        virtual bool callback_event_scroll(double dx, double dy);
        virtual void callback_event_resize(int w, int h);

        void draw_corner_axes();

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

		// mouse
		int		button_;		// for mouse drag
		int		modifiers_;		// for mouse drag
		bool	drag_active_;	// for mouse drag
		int		mouse_x_;		// for mouse button and scroll events
		int		mouse_y_;		
		int     mouse_pressed_x_;	// last pressed position
		int     mouse_pressed_y_;

		bool    show_pivot_point_;

		//----------------- viewer data -------------------

		// corner axes
        TrianglesDrawable* drawable_axes_;

        // camera path
        bool	show_camera_path_;

		std::vector<Model*> models_;
		int model_idx_;

        // drawables independent of any model
        std::vector<Drawable*> drawables_;
	};

}


#endif	// EASY3D_VIEWER_H
