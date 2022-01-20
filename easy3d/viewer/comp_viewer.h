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
    class VertexArrayObject;

    /// A composite viewer
    class CompViewer : public Viewer {
    public:
        class View {
        public:
            /// Assigns the model \p m to this view. After that, this view will visualize the model (regardless of
            /// other views visualizing the same model).
            void assign(const Model *m);

            /// Assigns the drawable \p d to this view. After that, this view will visualize the drawable (regardless
            /// of other views visualizing the same drawable).
            void assign(const Drawable *d);

        private:
            std::vector<const Model *> models_;     // the models to show in this view
            std::vector<const Drawable *> drawables_;  // the drawables to show in this view
            ivec4 viewport_;
            friend class CompViewer;
        };

    public:
        /// Constructor. \p rows and \cols together define the layout of the composite viewer.
        CompViewer(unsigned int rows, unsigned int cols, const std::string &title = "untitled");

        /// Returns a reference to the view at position (\p row, \p col).
        View& operator()(unsigned int row, unsigned int col) { return views_[row][col]; };
        /// Returns a constant reference to the view at position (\p row, \p col).
        const View& operator()(unsigned int row, unsigned int col) const { return views_[row][col]; };

    protected:
        void init() override;
        void post_resize(int w, int h) override;
        void cleanup() override;
        void draw() const override;

        void draw_borders() const;
        void update_borders();

    private:
        unsigned int num_rows_;
        unsigned int num_cols_;
        std::vector<std::vector<View> > views_;
        VertexArrayObject *borders_vao_;
        unsigned int borders_vertex_buffer_;
    };

}

#endif // EASY3D_VIEWER_COMP_VIEWER_H
