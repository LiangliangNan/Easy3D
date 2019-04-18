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


#ifndef EASY3D_AVERAGE_COLOR_BLENDING_H
#define EASY3D_AVERAGE_COLOR_BLENDING_H

#include <vector>
#include <easy3d/viewer/transparency.h>


namespace easy3d {

    class Camera;
    class TrianglesDrawable;
    class FramebufferObject;

    // Optimization tips: rendering with multi-effects (e.g., shadowing, SSAO)
    // can benefit from sharing the same geometry pass.

    class AverageColorBlending : public Transparency
    {
    public:
        AverageColorBlending(Camera* cam);
        ~AverageColorBlending() override;

        void draw(const std::vector<TrianglesDrawable*>& surfaces) override;

    protected:

        // Allocate and configure FBOs, shader programs.
        void init(int w, int h);

        // Render the opaque pass geometry.
        void geometry_pass(const std::vector<TrianglesDrawable*>& surfaces);

        // Release all FBOs, shader programs.
        void clear();

    private:
        Camera*	camera_;
        float   bkg_color_[4];

        FramebufferObject* fbo_;

    private:
        //copying disabled
        AverageColorBlending(const AverageColorBlending&);
        AverageColorBlending& operator=(const AverageColorBlending&);
    };

}

#endif // EASY3D_AVERAGE_COLOR_BLENDING_H
