#ifndef EASY3D_WXWIDGETS_VIEWER_H
#define EASY3D_WXWIDGETS_VIEWER_H

#include <easy3d/core/types.h>
#include <wx/glcanvas.h>


namespace easy3d {

    class Model;
    class Camera;
    class TrianglesDrawable;
    class TextRenderer;

    class Viewer : public wxGLCanvas {
    public:
        Viewer(wxWindow *parent, wxWindowID id = wxID_ANY,
               const wxPoint &pos = wxDefaultPosition,
               const wxSize &size = wxDefaultSize, long style = 0,
               const wxString &name = "Easy3D-Viewer-wxWidgets");

        virtual ~Viewer();

        virtual Model *add_model(const std::string &file_name, bool create_default_drawables = true);
        virtual Model *add_model(Model *model, bool create = true);

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
         * @brief Query the scaling factor for high DPI devices (e.g., MackBook pro).
         * @return The high DPI scaling factor.
         */
        double dpi_scaling() const;

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

        /**
         * @brief Delete all visual contents of the viewer (all models and drawables).
         */
        void clear_scene();

    protected:
        void OnPaint(wxPaintEvent &event);
        void OnSize(wxSizeEvent &event);
        void OnMouse(wxMouseEvent &event);

        // rendering. Users can put their additional rendering function here by reimplementing it.
        virtual void draw() const;

        // This function will be called before the main draw procedure.
        virtual void pre_draw();

        // This function draws axes of the coordinate system, Easy3D logo, frame rate, etc. overlaid on the scene.
        // It will be called after the main draw procedure.
        virtual void post_draw();

        void draw_corner_axes() const;

    private:
        void init();

    private:
        Camera*	camera_;

        wxGLContext *gl_contex_;
        bool initialized_;

        vec4	background_color_;
        TextRenderer* texter_;

        TrianglesDrawable* drawable_axes_;

        std::vector<Model*> models_;
        int model_idx_;

        wxDECLARE_NO_COPY_CLASS(Viewer);

        wxDECLARE_EVENT_TABLE();
    };

}

#endif // #ifndef EASY3D_WXWIDGETS_VIEWER_H
