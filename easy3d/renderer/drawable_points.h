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

#ifndef EASY3D_RENDERER_DRAWABLE_POINTS_H
#define EASY3D_RENDERER_DRAWABLE_POINTS_H

#include <easy3d/renderer/drawable.h>


namespace easy3d {


    /**
     * \brief The drawable for rendering a set of points, e.g., point clouds, vertices of a mesh.
     * \class PointsDrawable easy3d/renderer/drawable_points.h
     * \see LinesDrawable, TrianglesDrawable
     */

	class PointsDrawable : public Drawable {
	public:
        explicit PointsDrawable(const std::string& name = "", Model* model = nullptr);

        Type type() const override;

        // a point imposter can be a sphere, or a surfel/disc
        enum ImposterType {
            PLAIN,
            SPHERE,
            SURFEL
        };

        /** Get/Set impostor type. */
        ImposterType impostor_type() const { return impostor_type_; }
        void set_impostor_type(ImposterType t) { impostor_type_ = t; }

        /** Get/Set point size. */
        float point_size() const { return point_size_; }
        void set_point_size(float s) { point_size_ = s; }

        // Rendering.
        void draw(const Camera* camera) const override;

    protected:
        // without texture
        void _draw_plain_points(const Camera* camera) const;
        void _draw_spheres_sprite(const Camera* camera) const;
        void _draw_spheres_geometry(const Camera* camera) const;
        void _draw_surfels(const Camera* camera) const;

        // textured
        void _draw_plain_points_with_texture(const Camera* camera) const;
        void _draw_spheres_with_texture_sprite(const Camera* camera) const;
        void _draw_spheres_with_texture_geometry(const Camera* camera) const;
        void _draw_surfels_with_texture(const Camera* camera) const;

	private:
        float           point_size_;
        ImposterType    impostor_type_;
	};

}


#endif  // EASY3D_RENDERER_DRAWABLE_POINTS_H
