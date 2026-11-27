/** \cond LICENSE
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
 \endcond */

#ifndef EASY3D_VIEWER_MULTI_VIEWER_H
#define EASY3D_VIEWER_MULTI_VIEWER_H

#include <easy3d/viewer/viewer.h>

#include <memory>
#include <type_traits>


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
     * world_to_window() uses the active cell (initially the top-left cell and
     * changed by mouse interaction); window_to_world() selects the cell
     * containing its logical window position. File snapshots contain the grid
     * scene and do not include the ordinary Viewer HUD.
     * \see Viewer, ViewerBase, OffScreen
     */
    class MultiViewer : public Viewer {
    public:
        /**
         * \brief Create a multi-view grid in a viewer window.
         * \param rows Number of view rows; must be positive.
         * \param cols Number of view columns; must be positive.
         * \param title Window title.
         * \param window Initial size, multisample count and window flags, shared
         *        with Viewer - see \ref WindowOptions.
         * \param options Backend-specific Vulkan or OpenGL creation options.
         * \throws std::invalid_argument if rows or cols is not positive.
         */
        MultiViewer(int rows, int cols, const std::string &title = "Easy3D MultiViewer",
                          const WindowOptions& window = {},
#ifdef EASY3D_HAS_VULKAN
                          const VulkanOptions& options = {});
#else
                          const OpenGLOptions& options = {});
#endif

        /// Portable 2.8 construction with backend-neutral DeviceOptions.
        template <typename Options>
        MultiViewer(int rows, int cols, const std::string &title,
                    const WindowOptions &window, const Options &options,
                    typename std::enable_if<std::is_same<Options, graphics::DeviceOptions>::value>::type * = nullptr)
            : MultiViewer(rows, cols, title, window, detail::make_device_options(options))
        {
        }
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
         * \param samples Sample count of the offscreen target; 1 renders without
         *        multisampling, see \ref multisample_counts.
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
         * \brief Convert a world point to logical window coordinates via the active cell.
         * \param world World-space point.
         * \return Continuous logical window coordinates and normalized depth.
         * \details The active cell starts at the top-left and changes when a
         * cell is pressed. Points are not clipped to the selected cell.
         */
        vec3 world_to_window(const vec3& world) const override;

        /**
         * \brief Convert a logical window position to world coordinates via its grid cell.
         * \param window Logical x/y position from the top-left and normalized
         *        depth in [0, 1].
         * \return Corresponding world-space point.
         */
        vec3 window_to_world(const vec3& window) const override;

        /**
         * \brief Query the visible scene point beneath a logical grid pixel.
         * \details Depth is queried from the whole grid, then unprojected through
         *          the camera of the cell containing the requested position.
         */
        vec3 point_under_pixel(int x, int y, bool& found) const override;

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
        /// \name View hooks reimplemented for a grid
        //@{

        friend class Viewer;

        // Builds the grid cell viewports and the division lines once the backend
        // is ready; delegates to Viewer::init() first.
        void init() override;

        // Re-creates the grid cell viewports and division lines after the window
        // size changed.
        void post_resize(int w, int h) override;

        // Records the pressed cell as the active one, then lets Viewer handle the
        // camera interaction relative to that cell.
        bool mouse_press_event(int x, int y, int button, int modifiers) override;

        // Top-left corner of the active cell in logical window coordinates, so
        // Viewer rebases camera manipulation onto that cell.
        ivec2 interaction_origin() const override { return interaction_origin_; }

        // Rebuilds the grid division lines (Vulkan: cached draw commands; OpenGL:
        // the division drawable's geometry) after layout or size changes.
        void update_division();

        // Assembles every assigned cell's scene commands for the requested output
        // size, followed by the grid-division overlay.
        std::vector<graphics::DrawCommand> draw_commands(const Camera& camera, int width, int height,
                                                       std::vector<DrawSource>* sources, bool include_hidden_models = false) const override;

        bool read_scene_depth(int x, int y, const Camera& snapshot, float& depth) const override;

        // OpenGL compatibility path for unsupported effects and legacy queries.
#ifndef EASY3D_HAS_VULKAN
        bool supports_portable_rendering() const;
        void draw() const override;
        void draw_scene(const Camera& camera, const DrawSource* source = nullptr, bool guides = false) const override;
        void prepare_pick_view(int x, int y, int width, int height) const override;
#endif
        //@}

#ifdef EASY3D_HAS_VULKAN
        // Grid divisions cached as screen-space commands for one target size.
        mutable std::vector<graphics::DrawCommand> division_commands_;
        mutable int division_width_ = 0, division_height_ = 0;
#endif
        // Cell that currently receives camera drags; clamped to the grid when the
        // layout changes and updated when a mouse button is pressed in another cell.
        int active_row_ = 0, active_col_ = 0;
        // True while a snapshot renders: the cells then share the viewer's global camera
        // (whose projection is temporarily overridden) instead of their own.
        mutable bool snapshot_active_ = false;
        int num_rows_;    // grid size given to set_layout()
        int num_cols_;
        // A sub-view of the multi-view viewer.
        struct View {
            std::vector<const Model *> models;        // the models to show in this view
            std::vector<const Drawable *> drawables;  // the drawables to show in this view
            ivec4 viewport; // Vulkan: top-left pixels; OpenGL: bottom-left pixels.
        };
        // Cell contents, row-major: views_[row][col].
        std::vector<std::vector<View> > views_;

        // Origin of the cell where the ongoing drag started, so pointer coordinates are
        // translated into that cell's local space for picking and camera motion.
        ivec2 interaction_origin_ = ivec2(0);

        // OpenGL cell-division lines, rebuilt on resize; Vulkan keeps them as cached
        // draw commands (division_commands_) instead of a drawable.
        std::unique_ptr<LinesDrawable2D> drawable_division_;
    };

}
#endif	// EASY3D_VIEWER_MULTI_VIEWER_H
