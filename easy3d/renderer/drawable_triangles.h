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

#ifndef EASY3D_RENDERER_DRAWABLE_TRIANGLES_H
#define EASY3D_RENDERER_DRAWABLE_TRIANGLES_H

#include <easy3d/renderer/drawable.h>


namespace easy3d {


    /**
     * \brief The drawable for rendering a set of triangles, e.g., the surface of a triangular mesh.
     * \class TrianglesDrawable easy3d/renderer/drawable_triangles.h
     * \see LinesDrawable, PointsDrawable
     *
     * \note TrianglesDrawable supports triangles only. Visualizing general polygons typically requires tessellating
     * the faces into a set of triangles (using Tessellator or any other methods). Vertex coordinates and properties
     * (e.g., color, normal) must be provided as consecutive triplets in an array to be transferred to GPU.
     * See Drawable::update_vertex_buffer().
     */
    class TrianglesDrawable : public Drawable {
	public:
        explicit TrianglesDrawable(const std::string& name = "", Model* model = nullptr);

        Type type() const override;

        bool smooth_shading() const { return smooth_shading_; }
        void set_smooth_shading(bool b) { smooth_shading_ = b; }

        /**
         * @brief Query the opacity of the drawable, in the range [0.0, 1.0].
         * @return The opacity of the drawable.
         */
        float opacity() const { return opacity_; }

        /**
         * @brief Set the opacity of the drawable.
         * @param opacity The new opacity value, in the range [0.0, 1.0].
         */
        void set_opacity(float opacity) { opacity_ = opacity; }

        // Rendering.
        void draw(const Camera* camera) const override;

	private:
        bool    smooth_shading_;
        float   opacity_;
	};

}


#endif  // EASY3D_RENDERER_DRAWABLE_TRIANGLES_H
