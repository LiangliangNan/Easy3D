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

#ifndef EASY3D_TUTORIAL_TRANSPARENCY_H
#define EASY3D_TUTORIAL_TRANSPARENCY_H

#include <easy3d/viewer/viewer.h>


// This class renders a surface mesh with transparency effect using the 
// Average Color Blending technique.

namespace easy3d {
	class Transparency;
}

class TutorialTransparency : public easy3d::Viewer
{
public:
	TutorialTransparency(const std::string& title = "");

protected:
    bool key_press_event(int key, int modifiers) override;

    void draw() const override;

	void cleanup() override;

    std::string usage() const override;

private:
	easy3d::Transparency* transparency_;
	int method_;	// 0: disabled; 1: ACB; 2: DDP
};


#endif // EASY3D_TUTORIAL_TRANSPARENCY_H
