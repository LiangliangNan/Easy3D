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

#ifndef EASY3D_VIEWER_VIEWER_H
#define EASY3D_VIEWER_VIEWER_H


#include <string>
#include <vector>

#include <easy3d/core/types.h>


struct GLFWwindow;

namespace easy3d {

	class Camera;
	class Model;
    class Drawable;
    class TrianglesDrawable;
    class TextRenderer;
    class KeyFrameInterpolator;

    /**
     * @brief The built-in Easy3D viewer.
     * @class Viewer easy3d/viewer/viewer.h
     * @details Easy3D is really easy to use. That's why it has such a name.
     * To use the viewer, simply create an instance of Viewer, and call the run() method, e.g.,
     *      \code
     *          MyViewer viewer("Example");
     *          viewer.run();
     *      \endcode
     * The default Easy3D viewer can be easily extended by deriving your enhanced viewer from it
     * by re-implementing some of its unctions (see the documentation of Viewer).
     */
	class Viewer
	{
	public:
        /**
         * @brief Constructor
         * @param title The window title of the viewer, which can be changed by calling set_title() after construction.
         * @param samples The number of samples for multisample antialiasing
         * @param gl_major/gl_minor The OpenGL version to request. The created context will be
         *        compatible with the requested version (if the context creation succeeded).
         * @param full_screen Do you want the viewer to be fullscreen?
         * @param resizable Whether the viewer will be resizable by the user.
         * @param depth_bits The desired bit depths of the depth component of the default framebuffer.
         * @param stencil_bits The desired bit depths of the stencil component of the default framebuffer.
         * @param width The width of the viewer, which can be changed by calling resize() after construction.
         * @param height The height of the viewer, which can be changed by calling resize() after construction.
         */
		explicit Viewer(
			const std::string& title = "Easy3D Viewer",
			int samples = 4,
            int gl_major = 3,   // must >= 3
            int gl_minor = 2,   // must >= 2
            bool full_screen = false,
			bool resizable = true,
			int depth_bits = 24,
			int stencil_bits = 8,
			int width = 800,
			int height = 600
		);

        /**
         * @brief The destructor.
         */
		virtual ~Viewer();

        /**
         * @brief Run the viewer.
         * @param see_all See all contents within the screen? If true, the entire scene (i.e., all models) will be
         *      centered on the screen at a proper scale.
         */
        int run(bool see_all = true);

        /**
         * @brief Terminate the viewer.
         */
        virtual void exit();

        /// @name Viewer properties
        //@{

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

        /// @brief Returns the width of the viewer/window.
        int width() const { return width_; }
        /// @brief Returns the height of the viewer/window.
        int height() const { return height_; }
        /// @brief Queries the size of the viewer/window.
        void viewer_size(int& w, int& h) const { w = width_; h = height_; }

		/// @brief Returns the width of the framebuffer, which is identical to: width() * dpi_scaling().
		int framebuffer_width() const;
		/// @brief Returns the height of the framebuffer, which is identical to: height() * dpi_scaling().
		int framebuffer_height() const;
		/// @brief Queries the size of the framebuffer.
		void framebuffer_size(int& w, int& h) const;

        /**
         * @brief Sets the position of the content area of the viewer.
         * @details This function sets the position, in screen coordinates, of the upper-left corner of the content
         *      area of the viewer. If the viewer is a full screen window, this function does nothing.
         *  @param xpos The x-coordinate of the upper-left corner of the content area.
         *  @param ypos The y-coordinate of the upper-left corner of the content area.
         *  @note This function must only be called from the main thread.
         */
        void set_position(int xpos, int ypos);

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
        float dpi_scaling() const { return dpi_scaling_; }

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

        /// @brief Returns the camera used by the viewer. See \c Camera.
        Camera* camera() { return camera_; }
        /// @brief Returns the camera used by the viewer. See \c Camera.
        const Camera* camera() const { return camera_; }
        //@}

        /// @name File IO
        //@{

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
        //@}

        /// @name Model management
        //@{

        /**
         * @brief Add a model from a file to the viewer to be visualized. On success, the viewer
         *        will be in charge of the memory management of the model. The loaded model can be
         *        accessed by the 'current_model()' method.
         * @details This method loads a model into the viewer. It allows the user to control if
         *          default drawables will be created. The default drawables are
         *          - for point clouds: "vertices".
         *          - for surface meshes: "faces", "vertices", "edges", "borders".
         *          - for graphs: "vertices", "edges".
         *          These drawables are usually sufficient for basic rendering of the model. In case
         *          the default drawables don't meet the particular visualization purpose, you can
         *          override create_drawables() or set 'create_default_drawables' to false and create
         *          the drawables by calling Model::add_[type]_drawable()..
         * @param file_name The string of the file name.
         * @param create_default_drawables If true, the default drawables will be created.
         * @return The pointer to the model added to the viewer (nullptr if failed).
         * @related create_drawables(Model* model).
         */
        virtual Model* add_model(const std::string& file_name, bool create_default_drawables = true);

        /**
         * @brief Add an existing model to the viewer to be visualized. After a model being added
         *        to the viewer, the viewer will be in charge of its memory management.
         * @details This method adds a model into the viewer. It allows the user to control if
         *          default drawables will be created. The default drawables are
         *          - for point clouds: "vertices".
         *          - for surface meshes: "faces", "vertices", "edges", "borders".
         *          - for graphs: "vertices", "edges".
         *          These drawables are usually sufficient for basic rendering of the model. In case
         *          the default drawables don't meet the particular visualization purpose, you can
         *          override create_drawables() or set 'create_default_drawables' to false and create
         *          the drawables by calling Model::add_[type]_drawable()..
         * @param model The pointer to the model.
         * @param create_default_drawables If ture, the default drawables will be created.
         * @return The pointer to the model added to the viewer (nullptr if failed).
         * @related add_model(const std::string&, bool).
         */
        Model* add_model(Model* model, bool create_default_drawables = true);

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
         * @details The viewer can manage/visualize/process multiple models. The default behavior
         *          of the Easy3D viewer is, when a command is triggerred (e.g., the Save menu was
         *          clicked), only the active model is processed. This method is used to identify
         *          the active model.
         * @return The active model.
         */
        Model* current_model() const;
        //@}

        /// @name Drawable management
        //@{

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
        //@}

        /// @name UI-related functions
        //@{

        /**
         * @brief Update the display (i.e., repaint).
         * @details This method is used to update the display of the rendering. Client should call
         *          it when your data/view is changed.
         */
        void update() const;

        /**
         * @brief Moves the camera so that the entire scene or the active model is centered on the
         *        screen at a proper scale.
         * @param model The pointer to the model to be centered on the screen. If nullptr, the
         *        entire scene (i.e., all models) will be centered on the screen at a proper scale.
         */
        void fit_screen(const easy3d::Model* model = nullptr);

        /**
         * @brief Take a snapshot of the screen and save it to a file.
         * @details This method takes a snapshot of the screen and saves the snapshot into an image file.
         *          Internally, it will pop up a file dialog for specifying the file name.
         * @return true on success and false otherwise.
         */
	    bool snapshot() const;

        /**
         * @brief  Take a snapshot of the screen and save it to an image file. Supported image format: png, jpg, bmp, and tga.
         * @details This function renders the scene into a framebuffer and takes a snapshot of the framebuffer.
         *      It allow the snapshot image to have a dimension different from the viewer and it has no limit on the
         *      image size (if memory allows).
         * @param file_name The image file name.
         * @param scaling The scaling factor that determines the size of the image (default to 1.0, using the viewer size), i.e., 
         *      image_width = viewer_width * scaling;
         *      image_height = viewer_height * scaling;
         * @param samples The required number of samples for antialiased rendering (can be different from the default framebuffer).
         *      The default value is 0 (no antialiasing).
         * @param back_ground Determines the background color. 0: current color; 1: white; 2: transparent.
         * @param expand Expand the frustum to ensure the image aspect ratio.
         * @return true on success and false otherwise.
         */
        bool snapshot(const std::string& file_name, float scaling = 1.0f, int samples = 0, int back_ground = 1, bool expand = true) const;

        /**
         * @brief Query the XYZ coordinates of the surface point under the cursor.
         * @param x The cursor x-coordinate, relative to the left edge of the content area.
         * @param y The cursor y-coordinate, relative to the top edge of the content area.
         * @param found indicates whether the point was found or not.
         * @return The coordinates of the 3D point located at pixel (x,y) on screen. The returned point is valid only
         *      if found was returned true.
         * @attention The screen point (x, y) is expressed in the screen coordinate system with an origin in the
         *      upper left corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g., a
         *      Mac with a Retina display. If your inherited viewer uses a customized content area, you must also
         *      reimplement this function such that the x and y are relative to left and top edges of the content
         *      area, respectively.
         * @note This method assumes that a GL context is available, and that its content was drawn using the Camera
         *      (i.e. using its projection and model-view matrices). This method hence cannot be used for offscreen
         *      Camera computations. Use cameraCoordinatesOf() and worldCoordinatesOf() to perform similar operations
         *      in that case. The precision of the method highly depends on the z-Buffer, i.e., how the zNear() and
         *      zFar() values are fitted to your scene. Loose boundaries will result in imprecision along the viewing
         *      direction.
         */
		virtual vec3 point_under_pixel(int x, int y, bool &found) const;

        /**
         * @brief The usage information of the viewer. For the time being, it is the manual of this default viewer.
         */
        const std::string& usage() const { return usage_string_; }
        void set_usage(const std::string& usg) { usage_string_ = usg; }
	    //@}

        /// @name Algorithm execution
        //@{

        /**
         * @brief The keys. Currently only a limited number of commonly used keys are supported.
         */
        enum Key { // Do NOT modify the values!!!
            // the unknown key
            KEY_UNKNOWN = -1,
            // the number keys
            KEY_0 = 48, KEY_1 = 49, KEY_2 = 50, KEY_3 = 51, KEY_4 = 52, KEY_5 = 53, KEY_6 = 54, KEY_7 = 55,
            KEY_8 = 56, KEY_9 = 57,
            // the character keys
            KEY_A = 65, KEY_B = 66, KEY_C = 67, KEY_D = 68, KEY_E = 69, KEY_F = 70, KEY_G = 71, KEY_H = 72,
            KEY_I = 73, KEY_J = 74, KEY_K = 75, KEY_L = 76, KEY_M = 77, KEY_N = 78, KEY_O = 79, KEY_P = 80,
            KEY_Q = 81, KEY_R = 82, KEY_S = 83, KEY_T = 84, KEY_U = 85, KEY_V = 86, KEY_W = 87, KEY_X = 88,
            KEY_Y = 89, KEY_Z = 90,
            // the functional keys
            KEY_RIGHT = 262, KEY_LEFT = 263, KEY_DOWN = 264, KEY_UP = 265,
            KEY_F1 = 290,   KEY_F2 = 291,   KEY_F3 = 292,   KEY_F4 = 293,   KEY_F5 = 294,
            KEY_F6 = 295,   KEY_F7 = 296,   KEY_F8 = 297,   KEY_F9 = 298,
            // some printable keys
            KEY_SPACE = 32,
            KEY_COMMA = 44/* , */,  KEY_MINUS     = 45/* - */,  KEY_PERIOD = 46/* . */,
            KEY_SLASH = 47/* / */,  KEY_SEMICOLON = 59/* ; */,  KEY_EQUAL  = 61/* = */,
            KEY_LEFT_BRACKET = 91/* [ */,   KEY_BACKSLASH = 92/* \ */, KEY_RIGHT_BRACKET = 93/* ] */
        };

        /**
         * @brief The key modifiers. Currently only Shift, Ctrl, and Alt are supported.
         */
        enum Modifier { /* Do NOT modify the values!!! */
            MODIF_NONE = 0x0000, 
            MODIF_SHIFT = 0x0001, 
#ifdef __APPLE__    // To have the same shortcut behavior on macOS and other platforms (i.e., Windows and Linux)
            MODIF_CTRL = 0x0008,
#else
            MODIF_CTRL = 0x0002,
#endif
            MODIF_ALT = 0x0004
        };

        /**
         * @brief Mouse buttons. 
         */
        enum Button { // Do NOT modify the values!!!
            BUTTON_LEFT = 0,
            BUTTON_RIGHT = 1, 
            BUTTON_MIDDLE = 2
        };

        /**
         * A function type, which applies the user's algorithm(s)/operation(s) on the given model.
         * @param viewer A pointer to this viewer.
         * @param model The model to be operated on.
         * Example of defining such a function:
         *      @code
         *          bool reconstruct(Viewer* viewer, Model* model) {
         *              auto cloud = dynamic_cast<PointCloud *>(model);
         *              auto mesh = surface_reconstruction(cloud);
         *              if (!mesh)
         *                  return false;
         *              viewer->add_model(mesh);
         *              viewer->update();
         *              return true;
         *          }
         *      @endcode
         */
        using Function = std::function<bool(Viewer* viewer, Model* model)>;

        /**
         * @brief Bind a function that will be triggered by the shortcut 'modifier + key'.
         * @details This operation will overwrite the previous function (if exist) bound to the same key-modifier shortcut.
         * @param func The function to be executed, which will be triggered by the shortcut.
         * @param model The model to be processed.
         * @param key The shortcut key.
         * @param modifier The shortcut key modifier (e.g., Ctrl, Shift, Alt). This can be MODIF_NONE.
         * @sa Function
         */
        void bind(const Function& func, Model* model, Key key, Modifier modifier = MODIF_NONE) {
            commands_[key][modifier] = std::make_pair(func, model);
        }
        //@}

	    /// @name Animation
	    //@{
        /// @brief Function called at an equal interval for animation.
        std::function<bool(Viewer* viewer)> animation_func_;

        /// @brief Enable/Disable animation.
        /// @attention To have animation, \c animation_func_ must be provided to specify how scene geometry is modified.
        void set_animation(bool b);

        /// @brief Is animation currently being performed.
        bool is_animating() const;
        //@}

	protected:

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

		// To destroy OpenGL resources (e.g., shaders, textures, VAOs), there must exist a valid rendering context.
        // It is (usually) a bad idea to clean up OpenGL in a destructor because the OpenGL context may not exist
		// (e.g., destroyed already) or the visible one is not *current*. This cleanup() function is to ensure you
        // have a valid rendering context. See also init().
		// NOTE: Don't forget to call Viewer::cleanup() at the end of your inherited function.
		void cleanup();

		// This function will be called before the main draw procedure.
        virtual void pre_draw();

        // This function draws axes of the coordinate system, Easy3D logo, frame rate, etc. overlaid on the scene.
		// It will be called after the main draw procedure.
		virtual void post_draw();

		// External resize due to user interaction.
		// This function will be called after the window size being changed.
        // w/h: the new width and height of the viewer (not framebuffer, not viewport)
        virtual void post_resize(int w, int h) { (void)w, (void)h; }

		// Mouse button press event handler
		virtual bool mouse_press_event(int x, int y, int button, int modifiers);
		// Mouse button release event handler
		virtual bool mouse_release_event(int x, int y, int button, int modifiers);
		// Mouse drag (i.e., a mouse button was pressed) event handler
		virtual bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers);
		// Mouse free move (i.e., no mouse button was pressed) event handler
		virtual bool mouse_free_move_event(int x, int y, int dx, int dy, int modifiers);
		// Mouse scroll event handler
		virtual bool mouse_scroll_event(int x, int y, int dx, int dy);

		// Text input event handler: codepoint is native endian UTF-32 format
		// NOTE: This one reveals the actual character being sent (not just the physical key)
		virtual bool char_input_event(unsigned int codepoint);
		// Keyboard event handler.
		// NOTE: This function does not reveal the actual character.
		virtual bool key_press_event(int key, int modifiers);
		virtual bool key_release_event(int key, int modifiers);

		// Handle a file drop event
		virtual bool drop_event(const std::vector<std::string> & filenames);

		// Handle a focus change event
		virtual bool focus_event(bool focused);

	protected:
        GLFWwindow *create_window(const std::string &title,
                                  int samples,
                                  int gl_major,   // must >= 3
                                  int gl_minor,   // must >= 2
                                  bool full_screen,
                                  bool resizable,
                                  int depth_bits,
                                  int stencil_bits,
                                  int width,
                                  int height);

        void setup_callbacks(GLFWwindow*);

		/* Event handlers. Client code should not touch these */
        virtual bool callback_event_cursor_pos(double x, double y);
        virtual bool callback_event_mouse_button(int button, int action, int modifiers);
        virtual bool callback_event_keyboard(int key, int action, int mods);
        virtual bool callback_event_character(unsigned int codepoint);
        virtual bool callback_event_drop(int count, const char **filenames);
        virtual bool callback_event_scroll(double dx, double dy);
        virtual void callback_event_resize(int w, int h);

        void draw_corner_axes() const;

        void draw_face_labels(Model* model, TextRenderer* texter, int font_id, const vec3& color) const;
        void draw_vertex_labels(Model* model, TextRenderer* texter, int font_id, const vec3& color) const;

        void copy_view();
        void paste_view();

    protected:
		GLFWwindow*	window_;
		bool        should_exit_;
        float       dpi_scaling_;
        int         width_;
        int         height_;

		std::string	title_;
		Camera*		camera_;

        KeyFrameInterpolator* kfi_;
        bool    is_animating_;

        std::string usage_string_;

        int		samples_;	// the actual samples

		bool	full_screen_;
        vec4	background_color_;

		// enable/disable event processing
		bool	process_events_;
		char   gpu_time_[48];       // show the frame rate

        TextRenderer* texter_;

		// mouse
		int		pressed_button_;    // for mouse drag
		int		modifiers_;		    // for mouse drag
		bool	drag_active_;	    // for mouse drag
		int		mouse_current_x_;	// for mouse button and scroll events
		int		mouse_current_y_;
		int     mouse_pressed_x_;	// last pressed position
		int     mouse_pressed_y_;
		int     pressed_key_;

		bool    show_pivot_point_;
		bool    show_frame_rate_;

		//----------------- viewer data -------------------

		// corner axes
        TrianglesDrawable* drawable_axes_;

        // camera path
        bool	show_camera_path_;

		std::vector<Model*> models_;
		int model_idx_;

        // drawables independent of any model
        std::vector<Drawable*> drawables_;

        typedef std::pair<Function, Model*> FunctionModel;
        std::map<Key, std::map<Modifier, FunctionModel> >  commands_;
	};

}


#endif	// EASY3D_VIEWER_VIEWER_H
