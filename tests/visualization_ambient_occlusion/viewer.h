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

#ifndef EASY3D_TESTS_AMBIENT_OCCLUSION_H
#define EASY3D_TESTS_AMBIENT_OCCLUSION_H

#include <easy3d/viewer/viewer.h>


// This class renders a surface mesh with ambient occlusion to improve depth perception.

namespace easy3d {
	class AmbientOcclusion;
}


class TutorialAmbientOcclusion : public easy3d::Viewer
{
public:
    explicit TutorialAmbientOcclusion(const std::string& title = "");
    ~TutorialAmbientOcclusion() override;

protected:
    bool key_press_event(int key, int modifiers) override;
    void draw() const override;

private:
	easy3d::AmbientOcclusion* ao_;
	bool ao_enabled_;
};


#endif // EASY3D_TESTS_AMBIENT_OCCLUSION_H
