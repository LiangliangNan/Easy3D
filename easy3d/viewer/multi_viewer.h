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
     * \brief A viewer that displays assigned scene content in a grid of views.
     * \class MultiViewer easy3d/viewer/multi_viewer.h
     * \details MultiViewer shares a single viewer window and camera across a
     * regular rows-by-columns grid. Models and standalone drawables must first
     * be added to the viewer, then assigned to one or more cells with assign().
     * Adding an item alone does not make it visible in a cell. Inherited
     * scene-management methods still manage the viewer-wide collection.
     *
     * project_point() uses the active cell (initially the top-left cell and
     * changed by mouse interaction); unproject_point() selects the cell
     * containing its logical window position. File snapshots contain the grid
     * scene and do not include the ordinary Viewer HUD.
     */
    class MultiViewer : public Viewer {
    public:
        /**
         * \brief Create a multi-view grid in a viewer window.
         * \param rows Number of view rows; must be positive.
         * \param cols Number of view columns; must be positive.
         * \param title Window title.
         * \param width Initial logical window width.
         * \param height Initial logical window height.
         * \param samples Requested multisample count.
         * \param full_screen Whether to use fullscreen mode.
         * \param visible Whether the window is initially visible.
         * \param resizable Whether the user can resize the window.
         * \param options Backend-specific Vulkan or OpenGL creation options.
         * \throws std::invalid_argument if rows or cols is not positive.
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
         * \brief Add a managed model to a grid cell.
         * \details The model is drawn in this cell in addition to any content
         * already assigned there. It may also be assigned to other cells.
         * First add it to this viewer with add_model(); assignment does not
         * transfer ownership. Membership is a caller precondition and is not
         * checked by this method.
         * \param row The row index of the view.
         * \param col The column index of the view.
         * \param m The model to assign; it must be managed by this viewer.
         * \note Invalid coordinates or a null model are logged and ignored.
         */
        void assign(int row, int col, const Model *m);

        /**
         * \brief Add a managed standalone drawable to a grid cell.
         * \details The drawable is drawn in this cell in addition to any
         * already assigned content. It may also be assigned to other cells.
         * First add it with add_drawable(); assignment does not transfer
         * ownership. Membership is a caller precondition and is not checked.
         * \param row The row index of the view.
         * \param col The column index of the view.
         * \param d The drawable to assign; it must be managed by this viewer.
         * \note Invalid coordinates or a null drawable are logged and ignored.
         */
        void assign(int row, int col, const Drawable *d);

        /**
         * \brief Set whether grid division lines are visible.
         * \param b True to show the lines; false to hide them.
         */
        void set_division_visible(bool b);

        /**
         * \brief Query whether grid division lines are visible.
         * \return \c true if the splitting lines are visible, \c false otherwise.
         */
        bool division_visible() const;

        /// Return the number of rows in the grid.
        /// \return Grid row count.
        int rows() const { return num_rows_; }

        /// Return the number of columns in the grid.
        /// \return Grid column count.
        int columns() const { return num_cols_; }

        /**
         * \brief Set the grid dimensions.
         * \param rows Number of rows; must be positive.
         * \param cols Number of columns; must be positive.
         * \throws std::invalid_argument if either dimension is not positive.
         * \details Assignments in cells retained by the new dimensions remain
         * in place; cells removed by shrinking the grid are discarded.
         */
        void set_layout(int rows, int cols);

        /**
         * \brief Save the multi-view scene using a file-selection dialog.
         * \return True on success, false on cancellation or failure.
         */
        bool snapshot() const override;

        /**
         * \brief Render the grid scene to an image file.
         * \param filename Destination image path.
         * \param scaling Output-size multiplier relative to the viewer canvas.
         * \param samples Requested antialiasing sample count.
         * \param background Background mode: 0 uses the viewer background,
         *        1 uses white, 2 uses transparency.
         * \param expand If true, expand the view to match the output aspect ratio.
         * \return True on success, false if the image could not be written.
         * \details The saved image contains the assigned cell content and grid
         * divisions, but not the ordinary Viewer HUD.
         */
        bool snapshot(const std::string& filename, float scaling = 1, int samples = 4,
                      int background = 0, bool expand = true) const override;

        /**
         * \brief Project a world point using the active cell's camera viewport.
         * \param world World-space point.
         * \return Continuous logical window coordinates and normalized depth.
         * \details The active cell starts at the top-left and changes when a
         * cell is pressed. Points are not clipped to the selected cell.
         */
        vec3 project_point(const vec3& world) const override;

        /**
         * \brief Unproject a logical window position using its grid cell.
         * \param window Logical x/y position from the top-left and normalized
         *        depth in [0, 1].
         * \return Corresponding world-space point.
         */
        vec3 unproject_point(const vec3& window) const override;

        /**
         * \brief Remove a model and its assignments from every cell.
         * \param model Model to remove.
         * \return True if the model was managed by this viewer and removed.
         */
        bool delete_model(Model* model) override;

        /**
         * \brief Remove a standalone drawable and its assignments from every cell.
         * \param drawable Drawable to remove.
         * \return True if the drawable was managed by this viewer and removed.
         */
        bool delete_drawable(Drawable* drawable) override;

        /// Clear all cell assignments and all viewer-managed scene content.
        void clear_scene() override;

    protected:
        void init() override;
        void post_resize(int w, int h) override;
#ifndef EASY3D_HAS_VULKAN
        void draw() const override;
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
        void draw_scene(const Camera& camera, const DrawSource* source = nullptr, bool guides = false) const override;
        void prepare_pick_view(int x, int y, int width, int height) const override;
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
