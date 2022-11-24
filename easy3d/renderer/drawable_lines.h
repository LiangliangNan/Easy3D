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

#ifndef EASY3D_RENDERER_DRAWABLE_LINES_H
#define EASY3D_RENDERER_DRAWABLE_LINES_H

#include <easy3d/renderer/drawable.h>


namespace easy3d {

    /**
     * \brief The drawable for rendering a set of line segments, e.g., edges of a mesh, vector fields
     * \class LinesDrawable easy3d/renderer/drawable_lines.h
     * \see PointsDrawable, TrianglesDrawable
     */
	class LinesDrawable : public Drawable {
	public:
        explicit LinesDrawable(const std::string& name = "", Model* model = nullptr);

        Type type() const override;

        // a line poster can be a cylinder, or a cone
        enum ImposterType {
            PLAIN,
            CYLINDER,
            CONE
        };
        ImposterType impostor_type() const { return impostor_type_; }
        void set_impostor_type(ImposterType t) { impostor_type_ = t; }

        float line_width() const { return line_width_; }
        void set_line_width(float w) { line_width_ = w; }

        // Rendering.
        void draw(const Camera* camera) const override;

    protected:
        // without texture
        void _draw_plain_lines(const Camera* camera) const;
        void _draw_cylinders(const Camera* camera) const;
        void _draw_cones(const Camera* camera) const;

        // textured
        void _draw_plain_lines_with_texture(const Camera* camera) const;
        void _draw_cylinders_with_texture(const Camera* camera) const;
        void _draw_cones_with_texture(const Camera* camera) const;

    private:
        float           line_width_;
        ImposterType    impostor_type_;
    };

}


#endif  // EASY3D_RENDERER_DRAWABLE_LINES_H
