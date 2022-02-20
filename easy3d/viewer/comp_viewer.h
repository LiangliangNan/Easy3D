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

#ifndef EASY3D_VIEWER_COMP_VIEWER_H
#define EASY3D_VIEWER_COMP_VIEWER_H

#include <easy3d/viewer/viewer.h>


namespace easy3d {

    class Model;
    class Drawable;
    class ShaderProgram;
    class VertexArrayObject;

    /**
     * @brief A composite viewer, which supports multiple views (arranged in a grid layout) in the same viewer.
     * @class CompViewer easy3d/viewer/comp_viewer.h
     */
    class CompViewer : public Viewer {
    public:
        /**
         * @brief Constructor. \p rows and \p cols together define the layout of the composite viewer.
         */
        CompViewer(unsigned int rows, unsigned int cols, const std::string &title = "untitled");

        /**
         * @brief Assigns the model \p m to the view at position (\p row, \p col).
         * @details This function assigns the model \p m to the view at position (\p row, \p col). After that, the
         *     model will be visualized in this view (regardless of other views visualizing the same model).
         * @note By default, a model added to the viewer through add_model() is not assigned to any view for
         *      visualization. However, add_model() allows the viewer to take ownership of the model, so memory
         *      management and drawable creation for the model are handled by the viewer.
         */
        void assign(unsigned int row, unsigned int col, const Model *m);

        /** @brief Assigns the drawable \p d to the view at position (\p row, \p col).
         *  @details This function assigns the drawable \p d to the view at position (\p row, \p col). After that, the
         *      drawable will be visualized in this view (regardless of other views visualizing the same drawable).
         * @note By default, a drawable added to the viewer through add_drawable() is not assigned to any view for
         *      visualization. However, add_drawable() allows the viewer to take ownership of the drawable, so its
         *      memory management is handled by the viewer.
         */
        void assign(unsigned int row, unsigned int col, const Drawable *d);

        /// Sets the visibility of the splitting lines of the views (visible by default).
        void set_division_visible(bool b) { division_visible_ = b; }

        /// Returns if the splitting lines of the views are visible.
        bool division_visible() const { return division_visible_; }

    protected:
        void init() override;
        void post_resize(int w, int h) override;
        void cleanup() override;
        void draw() const override;

        // overloaded, so mouse positions are relative to the current view
        bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;

        void draw_division() const;
        void update_division();

    private:
        unsigned int num_rows_;
        unsigned int num_cols_;
        // A sub-view of the composite viewer.
        struct View {
            std::vector<const Model *> models;        // the models to show in this view
            std::vector<const Drawable *> drawables;  // the drawables to show in this view
            ivec4 viewport;
        };
        std::vector<std::vector<View> > views_;
        VertexArrayObject *division_vao_;
        ShaderProgram *lines_program_;
        unsigned int division_vertex_buffer_;
        bool division_visible_;

        int view_width_;    // the width of the views
        int view_height_;   // the height of the views
    };

}

#endif // EASY3D_VIEWER_COMP_VIEWER_H
