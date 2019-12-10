/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef EASY3D_DRAWABLE_LINES_H
#define EASY3D_DRAWABLE_LINES_H

#include <easy3d/viewer/drawable.h>


namespace easy3d {


    // The drawable for rendering a set of line segments, e.g., wireframe of a mesh, vector fields
	class LinesDrawable : public Drawable {
	public:
        LinesDrawable(const std::string& name = "")
            : Drawable(name)
            , impostors_(false)
            , impostor_thickness_(1.0f)
        {
			default_color_ = vec3(0.0f, 0.0f, 0.0f);
		}

        DrawableType type() const override;

        bool impostors() const { return impostors_; }
        void set_impostors(bool b) { impostors_ = b; }

        float impostor_thickness() const { return impostor_thickness_; }
        void set_impostor_thickness(float t) { impostor_thickness_ = t; }

        // Rendering.
        virtual void draw(const Camera* camera, bool with_storage_buffer = false) const override;

    private:
        bool  impostors_;
        float impostor_thickness_;
    };

}


#endif  // EASY3D_DRAWABLE_LINES_H
