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
    class VertexArrayObject;
}

/// A composite viewer
class CompViewer : public easy3d::Viewer
{
public:
    struct View {
        std::vector<easy3d::Model*>     models;     // the models to show in this view
        std::vector<easy3d::Drawable*>  drawables;  // the drawables to show in this view
        easy3d::ivec4 viewport;
    };

public:
    CompViewer(const std::string& title, int num_rows = 1, int num_cols = 2);

    View& view(int row, int column) { return views_[row][column]; };
    const View& view(int row, int column) const { return views_[row][column]; };

protected:
    void draw() const override;

    void init() override;

    // This function will be called after the window size being changed.
    void post_resize(int w, int h) override;

    void cleanup() override ;

    void draw_view_borders() const;

    void update_view_borders();

private:
    unsigned int num_rows_;
    unsigned int num_cols_;
    std::vector< std::vector<View> > views_;
    easy3d::VertexArrayObject* borders_;
    unsigned int borders_vertex_buffer_;
};


#endif // EASY3D_VIEWER_COMP_VIEWER_H
