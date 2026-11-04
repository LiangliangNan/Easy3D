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

#ifndef EASY3D_VIEWER_MULTI_VIEWER_H
#define EASY3D_VIEWER_MULTI_VIEWER_H

#include <easy3d/viewer/viewer.h>

#include <memory>


namespace easy3d {

    class Model;
    class LinesDrawable2D;

    /**
     * \brief A viewer that supports multiple views (arranged in a grid layout).
     * \class MultiViewer easy3d/viewer/multi_viewer.h
     */
    class MultiViewer : public Viewer {
    public:
        // ... some public functions: the unified APIs used by both backends
        /**
         * \brief Constructor.
         * \param rows The number of rows (of the grid-like layout).
         * \param cols The number of columns (of the grid-like layout).
         * \param title The title of the viewer window.
         */
        MultiViewer(int rows, int cols, const std::string &title = "untitled",
                          int width = 800, int height = 600, int samples = 1,
                          bool full_screen = false, bool visible = true, bool resizable = true,
#ifdef EASY3D_HAS_VULKAN
                          const VulkanOptions& options = {});
#else
                          const OpenGLOptions& options = {});
#endif
        /**
         * \brief Destructor.
         */
        ~MultiViewer() override;

        /**
         * \brief Assigns the model \p m to the view at position (\p row, \p col).
         * \details This function assigns the model \p m to the view at position (\p row, \p col). After that, the
         *      model will be visualized in this view (regardless of other views visualizing the same model).
         * \note You need to first add the model to the viewer by calling add_model(). The add_model() function
         *      does not assign the model to any view for rendering. However, add_model() allows the viewer to
         *      take ownership of the model.
         * \param row The row index of the view.
         * \param col The column index of the view.
         * \param m The model to assign.
         */
        void assign(int row, int col, const Model *m);

        /**
         * \brief Assigns the drawable \p d to the view at position (\p row, \p col).
         * \details This function assigns the drawable \p d to the view at position (\p row, \p col). After that, the
         *      drawable will be visualized in this view (regardless of other views visualizing the same drawable).
         * \note You need to first add the drawable to the viewer by calling add_drawable(). The add_drawable()
         *      function does not assign the drawable to any view for rendering. However, add_drawable() allows the
         *      viewer to take ownership of the drawable.
         * \param row The row index of the view.
         * \param col The column index of the view.
         * \param d The drawable to assign.
         */
        void assign(int row, int col, const Drawable *d);

        /**
         * \brief Sets the visibility of the splitting lines of the views.
         * \param b Whether the splitting lines are visible.
         */
        void set_division_visible(bool b);

        /**
         * \brief Returns if the splitting lines of the views are visible.
         * \return \c true if the splitting lines are visible, \c false otherwise.
         */
        bool division_visible() const;

        /**
         * \brief Return the number of rows (of the grid-like layout) of the viewer.
         * \return The number of rows.
         */
        int rows() const { return num_rows_; }

        /**
         * \brief Return the number of columns (of the grid-like layout) of the viewer.
         * \return The number of columns.
         */
        int columns() const { return num_cols_; }

        /**
         * \brief Set/Change the layout of the viewer.
         * \param rows The number of rows (of the grid-like layout).
         * \param cols The number of columns (of the grid-like layout).
         */
        void set_layout(int rows, int cols);

        /**
         * \brief Take a snapshot of the screen and save it to a file.
         * \details This method takes a snapshot of the screen and saves the snapshot into an image file.
         *          Internally, it will pop up a file dialog for specifying the file name.
         * \return \c true on success and \c false otherwise.
         */
        bool snapshot() const override;
        bool snapshot(const std::string& filename, float scaling = 1, int samples = 4,
                      int background = 1, bool expand = true) const override;
        // Projects into the last pressed cell (initially the top-left cell).
        // Unprojection resolves the cell from the canvas coordinate.
        vec3 project_point(const vec3& world) const override;
        vec3 unproject_point(const vec3& window) const override;
        bool delete_model(Model* model) override;
        bool delete_drawable(Drawable* drawable) override;
        void clear_scene() override;

    protected:
        void init() override;
        void post_resize(int w, int h) override;
#ifndef EASY3D_HAS_VULKAN
        bool draw() override;
#endif

        bool mouse_press_event(int x, int y, int button, int modifiers) override;
#ifdef EASY3D_HAS_VULKAN
        // Keep this part of code minimum
        std::vector<graphics::DrawCommand> draw_commands(const Camera& camera, int width, int height,
                                                       std::vector<DrawSource>* sources, bool include_hidden_models = false) const override;
        mutable std::vector<graphics::DrawCommand> division_commands_;
        mutable int division_width_ = 0, division_height_ = 0;
#else
        // Keep this part of code minimum
        void draw_scene_depth(const Camera& camera, const DrawSource* source = nullptr) const override;
        // overloaded, so mouse positions are relative to the current view
        bool mouse_release_event(int x, int y, int button, int modifiers) override;
        // overloaded, so mouse positions are relative to the current view
        bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;

#endif
        void update_division();

    protected:
        int active_row_ = 0, active_col_ = 0;
        mutable bool snapshot_active_ = false;
        int num_rows_;
        int num_cols_;
        // A sub-view of the multi-view viewer.
        struct View {
            std::vector<const Model *> models;        // the models to show in this view
            std::vector<const Drawable *> drawables;  // the drawables to show in this view
            ivec4 viewport; // Vulkan: top-left pixels; OpenGL: bottom-left pixels.
        };
        std::vector<std::vector<View> > views_;

        int view_width_;    // the width of the views
        int view_height_;   // the height of the views

        std::unique_ptr<LinesDrawable2D> drawable_division_;
    };

}
#endif	// EASY3D_VIEWER_MULTI_VIEWER_H
