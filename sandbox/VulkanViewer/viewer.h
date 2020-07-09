#ifndef EASY3D_VIEWER_VULKAN_H
#define EASY3D_VIEWER_VULKAN_H

#include "application.h"


namespace easy3d {

    class Model;
    class Camera;

    class ViewerVK : public ApplicationVulkan {
    public:
        ViewerVK(const std::string& title = "Easy3dViewer");

        /**
         * Run the viewer.
         */
        void run() override ;

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

//        /**
//         * @brief Query the actual samples of the viewer.
//         * @note The requested sample may not be supported by the context or graphics driver. This
//         *       method returns the actual samples that the viewer supports.
//         */
//        int	 samples() const { return samples_; }
//
//        /**
//         * @brief Query the scaling factor for high DPI devices (e.g., MackBook pro).
//         * @return The high DPI scaling factor.
//         */
//        double dpi_scaling() const { return dpi_scaling_; }

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
         *          - for point clouds: "vertices".
         *          - for surface meshes: "faces", "vertices", "edges", "borders".
         *          - for graphs: "vertices", "edges".
         *          These drawables are usually sufficient for basic rendering of the model. In case
         *          the default drawables don't meet the particular visualization purpose, you can
         *          override create_drawables() or set 'create_default_drawables' to false and create
         *          the drawables by calling Model::add_[type]_drawable()..
         * @param file_name The string of the file name.
         * @param create_default_drawables If ture, the default drawables will be created.
         * @return The pointer to the model added to the viewer (nullptr if failed).
         * @related create_drawables(Model* model).
         */
        virtual Model* add_model(const std::string& file_name, bool create_default_drawables = true);

        /**
         * @brief Add an existing model to the viewer to be visualized. After a model being added
         *        to the viewer, the viewer will be incharge of its memory menagement.
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


        /**
         * @brief Moves the camera so that the entire scene or the active model is centered on the
         *        screen at a proper scale.
         * @param model The pointer to the model to be centered on the screen. If nullptr, the
         *        entire scene (i.e., all models) will be centered on the screen at a proper scale.
         */
        virtual void fit_screen(const easy3d::Model* model = nullptr);


        /**
         * @brief Update the display (i.e., repaint).
         * @details This method is used to update the display of the rendering. Client should call
         *          it when your data/view is changed.
         */
        void update() const;

    protected:

        // NOTE: Don't forget to call Viewer::initialize() at the beginning of your inherited function.
        virtual void initialize();

        // NOTE: Don't forget to call Viewer::initialize() at the end of your inherited function.
        virtual void cleanup() override ;

        void mainLoop() override ;

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

        // updated camera
        void updateUniformBuffer(uint32_t currentImage) override ;

    protected:
        void setup_callbacks(GLFWwindow*);

        /* Event handlers. Client code should not touch these */
        bool callback_event_cursor_pos(double x, double y);
        bool callback_event_mouse_button(int button, int action, int modifiers);
        bool callback_event_keyboard(int key, int action, int mods);
        bool callback_event_character(unsigned int codepoint);
        bool callback_event_drop(int count, const char **filenames);
        bool callback_event_scroll(double dx, double dy);
        void callback_event_resize(int w, int h);

    private:
        std::string	title_;
        Camera*		camera_;

        vec4	background_color_;

        // enable/disable event processing
        bool	process_events_;

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

        std::vector<Model*> models_;
        int model_idx_;
    };

}   // namespace easy3d


#endif  // EASY3D_VIEWER_VULKAN_H