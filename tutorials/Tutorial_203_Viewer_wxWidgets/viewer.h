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

#ifndef EASY3D_TUTORIAL_VIEWER_WXWIDGETS_VIEWER_H
#define EASY3D_TUTORIAL_VIEWER_WXWIDGETS_VIEWER_H

#include <easy3d/core/types.h>
#include <wx/glcanvas.h>


namespace easy3d {

    class Model;
    class Camera;
    class Drawable;
    class TrianglesDrawable;
    class TextRenderer;

    class Viewer : public wxGLCanvas {
    public:
        Viewer(wxWindow *parent,
               const wxGLAttributes& glAttrs,
               wxWindowID id = wxID_ANY,
               const wxPoint &pos = wxDefaultPosition,
               const wxSize &size = wxDefaultSize, long style = 0,
               const wxString &title = "Easy3D-Viewer-wxWidgets");

        ~Viewer() override;

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
        virtual Model* add_model(const std::string& file_name, bool create_default_drawables);

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
        virtual Model* add_model(Model* model, bool create_default_drawables);

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

        /**
         * @brief Save the active model (if exists) to a file.
         * @details This method saves the active model to a file.
         * @return true on success and false otherwise.
         */
        bool save_current_model(const std::string& file_name) const;

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
        float dpi_scaling() const;

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
         * @brief Take a snapshot of the screen and save it to an image file.
         * @param image_file the full path to the image file.
         * @param bk_white true to have a white background.
         * @return true on success and false otherwise.
         */
        bool snapshot(const std::string& image_file, bool bk_white = true) const;

    protected:
        void OnPaint(wxPaintEvent &event);
        void OnSize(wxSizeEvent &event);
        void OnMouse(wxMouseEvent &event);
        void OnKeyDown(wxKeyEvent &event);

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

        wxGLContext *gl_context_;
        bool initialized_;

        vec4	background_color_;
        TextRenderer* texter_;

        TrianglesDrawable* drawable_axes_;

        std::vector<Model*> models_;
        int model_idx_;

        // drawables independent of any model
        std::vector<Drawable*> drawables_;

        wxDECLARE_NO_COPY_CLASS(Viewer);
        wxDECLARE_EVENT_TABLE();
    };

}

#endif // EASY3D_TUTORIAL_VIEWER_WXWIDGETS_VIEWER_H
