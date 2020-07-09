/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
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
 */

#ifndef EASY3D_TUTORIAL_SOFT_SHADOW_H
#define EASY3D_TUTORIAL_SOFT_SHADOW_H

#include <easy3d/viewer/viewer.h>


// This class renders a scene with soft shadow using the 
// Percentage-Closer Soft Shadows (PCSS) technique.

namespace easy3d {
	class Shadow;
}

class TutorialSoftShadow : public easy3d::Viewer
{
public:
	TutorialSoftShadow(const std::string& title = "");

protected:
    bool key_press_event(int key, int modifiers) override;

    void draw() const override;

	void cleanup() override;

    std::string usage() const override;

private:
	easy3d::Shadow* shadow_;
	bool			shadow_enabled_;
};


#endif // EASY3D_TUTORIAL_SOFT_SHADOW_H
