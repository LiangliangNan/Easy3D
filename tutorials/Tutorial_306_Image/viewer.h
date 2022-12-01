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

#ifndef EASY3D_TUTORIAL_IMAGE_VIEWER_H
#define EASY3D_TUTORIAL_IMAGE_VIEWER_H

#include <easy3d/viewer/viewer.h>


// This class visualizes an image in the screen space.

namespace easy3d {
    class Texture;
}

class ImageViewer : public easy3d::Viewer
{
public:
    ImageViewer(const std::string& title, const std::string& image_file);
    ~ImageViewer() override;
    
protected:
    bool key_press_event(int key, int modifiers) override;
    bool mouse_scroll_event(int x, int y, int dx, int dy) override;
    void init() override;

    void draw() const override;

    void compute_image_region(int& x, int& y, int& w, int& h) const;

    // moves the camera so that the 'model' is centered on the screen.
    // if 'model' is NULL, it centers the entire scene (all models).
    void fit_screen();

private:
    easy3d::Texture* texture_;
    std::string      image_file_;
    float            scale_;
};


#endif // EASY3D_TUTORIAL_IMAGE_VIEWER_H
