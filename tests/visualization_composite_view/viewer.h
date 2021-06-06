/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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

#ifndef EASY3D_TUTORIAL_COMPOSITE_VIEW_H
#define EASY3D_TUTORIAL_COMPOSITE_VIEW_H

#include <easy3d/viewer/viewer.h>


// This example uses a "split window" view, rendering four views of the
// same scene in one window (e.g. uesful for 3D modelling software). This
// demo uses scissors to separate the four different rendering areas from
// each other.

namespace easy3d {
    class LinesDrawable;
}

class CompositeView : public easy3d::Viewer
{
public:
    CompositeView(const std::string& title);

protected:
    void draw() const override;

    // This function will be called after the window size being changed.
    void post_resize(int w, int h) override;

    void cleanup() override ;

    void draw_grid() const;

    void update_grid();

private:
    // Active view: -1: none, 0: upper left, 1: upper right, 2: lower left, 3: lower right
    int grid_size_;
    easy3d::LinesDrawable* grid_;
};


#endif // EASY3D_TUTORIAL_COMPOSITE_VIEW_H
