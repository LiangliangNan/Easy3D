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

#include <memory>
#include <easy3d/viewer/viewer.h>


namespace easy3d {

    class Model;
    class LinesDrawable2D;

    /**
     * \brief A viewer that supports multiple views (arranged in a grid layout).
     * \class MultiViewer easy3d/viewer/multi_viewer.h
     */
    class MultiViewer : public Viewer {
    public:
        /**
         * \brief Constructor.
         * \param rows The number of rows (of the grid-like layout).
         * \param cols The number of columns (of the grid-like layout).
         * \param title The title of the viewer window.
         */
        MultiViewer(int rows, int cols, const std::string &title = "untitled");
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
         * \brief Query the XYZ coordinates of the surface point under the cursor.
         * \param x The cursor x-coordinate, relative to the left edge of the content area.
         * \param y The cursor y-coordinate, relative to the top edge of the content area.
         * \param found Indicates whether the point was found or not.
         * \return The coordinates of the 3D point located at pixel (x, y) on screen. The returned point is valid
         *      only if \p found is \c true.
         * \attention The screen point (x, y) is expressed in the screen coordinate system with an origin in the
         *      upper left corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g., a
         *      Mac with a Retina display. If your inherited viewer uses a customized content area, you must also
         *      reimplement this function such that the x and y are relative to left and top edges of the content
         *      area, respectively.
         */
        vec3 point_under_pixel(int x, int y, bool &found) const override;

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

    protected:
        void init() override;
        void post_resize(int w, int h) override;
        void draw() const override;

        // overloaded, so mouse positions are relative to the current view
        bool mouse_release_event(int x, int y, int button, int modifiers) override;
        // overloaded, so mouse positions are relative to the current view
        bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;

        void update_division();

    protected:
        int num_rows_;
        int num_cols_;
        // A sub-view of the multi-view viewer.
        struct View {
            std::vector<const Model *> models;        // the models to show in this view
            std::vector<const Drawable *> drawables;  // the drawables to show in this view
            ivec4 viewport;
        };
        std::vector<std::vector<View> > views_;

        int view_width_;    // the width of the views
        int view_height_;   // the height of the views

        std::unique_ptr<LinesDrawable2D> drawable_division_;
    };

}

#endif // EASY3D_VIEWER_MULTI_VIEWER_H
