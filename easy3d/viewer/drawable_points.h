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

#ifndef EASY3D_DRAWABLE_POINTS_H
#define EASY3D_DRAWABLE_POINTS_H

#include <easy3d/viewer/drawable.h>


namespace easy3d {


    // The drawable for rendering a set of points, e.g., point clouds, vertices of a mesh
	class PointsDrawable : public Drawable {
	public:
        PointsDrawable(const std::string& name = "");

        DrawableType type() const override;

        // default_color will be ignored if per_vertex_color is true and given.
		float point_size() const { return point_size_; }
		void set_point_size(float s) { point_size_ = s; }

        // if ture, points are renderred as shperes
        bool impostors() const { return impostors_; }
        void set_impostors(bool b) { impostors_ = b; }

        // Rendering.
        virtual void draw(const Camera* camera, bool with_storage_buffer = false) const override;

    protected:
        // without texture
        void _draw_plain_points(const Camera* camera, bool with_storage_buffer) const;
        void _draw_sprite_spheres(const Camera* camera, bool with_storage_buffer) const;
        void _draw_geometry_spheres(const Camera* camera, bool with_storage_buffer) const;

        // textured
        void _draw_plain_points_with_texture(const Camera* camera, bool with_storage_buffer) const;
        void _draw_sprite_spheres_with_texture(const Camera* camera, bool with_storage_buffer) const;
        void _draw_geometry_spheres_with_texture(const Camera* camera, bool with_storage_buffer) const;

	private:
        float       point_size_;
        bool        impostors_;
	};

}


#endif  // EASY3D_DRAWABLE_POINTS_H
