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

#ifndef EASY3D_TUTORIAL_EYE_DOME_LIGHTING_H
#define EASY3D_TUTORIAL_EYE_DOME_LIGHTING_H

#include <easy3d/viewer/viewer.h>


// This class renders a point cloud without normal information. 
// It uses the Eye Dome Lighting technique to improve depth perception.

namespace easy3d {
	class EyeDomeLighting;
}

class TutorialEyeDomeLighting : public easy3d::Viewer
{
public:
	explicit TutorialEyeDomeLighting(const std::string& title = "");
    ~TutorialEyeDomeLighting() override;

protected:
    bool key_press_event(int key, int modifiers) override;

    void draw() const override;

private:
	easy3d::EyeDomeLighting* edl_;
	bool edl_enabled_;
};


#endif // EASY3D_TUTORIAL_EYE_DOME_LIGHTING_H
