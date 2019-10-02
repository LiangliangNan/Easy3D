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

#ifndef EASY3D_MODEL_H
#define EASY3D_MODEL_H


#include <string>
#include <vector>

#include <easy3d/core/types.h>


namespace easy3d {

    class PointsDrawable;
    class LinesDrawable;
    class TrianglesDrawable;

    /*
    * Model is the base class for 3D models, e.g., SurfaceMesh, PointCloud.
    * A Model can have multiple drawables, e.g., faces, vertices, wireframe.
    */
    class Model
    {
    public:
        Model(const std::string& name = "unknown");
        virtual ~Model();

        const std::string& name() const { return name_; }
        void set_name(const std::string& n) { name_ = n; }

        const Box3& bounding_box() const;
        void set_bounding_box(const Box3& box);

        /// returns number of vertices in the model
        virtual unsigned int n_vertices() const = 0;

        //-------------------- rendering  -----------------------

        bool is_visible() const { return visible_; }
        void set_visible(bool b) { visible_ = b; }

        // Gets the drawable named 'name'.
        // Returns a nullptr if the drawable does not exist.
        PointsDrawable* points_drawable(const std::string& name) const;
        LinesDrawable*  lines_drawable(const std::string& name) const;
        TrianglesDrawable*  triangles_drawable(const std::string& name) const;

        // Create a drawable named 'name'.
        // Ignore the creation if a drawable named 'name' already exists.
        PointsDrawable* add_points_drawable(const std::string& name);
        LinesDrawable*  add_lines_drawable(const std::string& name);
        TrianglesDrawable*  add_triangles_drawable(const std::string& name);

        // Returns all available drawables.
        const std::vector<PointsDrawable*>&  points_drawables() const { return points_drawables_; }
        const std::vector<LinesDrawable*>&   lines_drawables() const { return lines_drawables_; }
        const std::vector<TrianglesDrawable*>& triangles_drawables() const { return triangles_drawables_; }

    protected:
        std::string	name_;
        bool		visible_;
        Box3		bbox_;
        bool		bbox_known_;

        std::vector<PointsDrawable*>    points_drawables_;
        std::vector<LinesDrawable*>     lines_drawables_;
        std::vector<TrianglesDrawable*> triangles_drawables_;
    };
}

#endif  // EASY3D_MODEL_H
