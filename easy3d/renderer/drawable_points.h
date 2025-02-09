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
     * \sa LinesDrawable, TrianglesDrawable
     */

	class PointsDrawable : public Drawable {
	public:
		/**
		 * \brief Constructor that initializes the drawable with a name and an optional model.
		 * \param name The name of the drawable.
		 * \param model The model to which the drawable is attached (can be nullptr).
		 */
		explicit PointsDrawable(const std::string& name = "", Model* model = nullptr);

		/**
		 * \brief Returns the type of the drawable.
		 * \return The type of the drawable, which is DT_POINTS.
		 */
		Type type() const override { return DT_POINTS; }

		/**
		 * \brief The type of the point imposter.
		 */
        enum ImposterType {
            PLAIN,	//!< Plain mode.
            SPHERE, //!< The points will be drawn as spheres.
            SURFEL	//!< The points will be drawn as surfels/discs.
        };

		/**
		 * \brief Returns the type of the point imposter.
		 * \return The type of the point imposter.
		 */
        ImposterType impostor_type() const { return impostor_type_; }
		/**
		 * \brief Sets the type of the point imposter.
		 * \param t The new type of the point imposter.
		 */
        void set_impostor_type(ImposterType t) { impostor_type_ = t; }

		/**
		 * \brief Returns the size of the points.
		 * \return The size of the points.
		 */
        float point_size() const { return point_size_; }
		/**
		 * \brief Sets the size of the points.
		 * \param s The new size of the points.
		 */
        void set_point_size(float s) { point_size_ = s; }

		/**
		 * \brief Draws the drawable.
		 * \param camera The camera used for rendering.
		 */
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
