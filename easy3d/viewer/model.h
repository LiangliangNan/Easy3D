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

#ifndef EASY3D_MODEL_H
#define EASY3D_MODEL_H


#include <string>
#include <vector>
#include <unordered_map>

#include <easy3d/core/types.h>


namespace easy3d {

    class Drawable;
    class PointsDrawable;
    class LinesDrawable;
    class TrianglesDrawable;

    /*
    * Model is the base class for 3D models, e.g., SurfaceMesh, PointCloud.
    * A Model can have multiple drawables, e.g., faces, vertices, edges.
    */
    class Model
    {
    public:
        Model(const std::string& name = "unknown");
        virtual ~Model();

        const std::string& name() const { return name_; }
        void set_name(const std::string& n) { name_ = n; }

        virtual const Box3& bounding_box() const;

        /// returns number of vertices in the model
        virtual unsigned int n_vertices() const = 0;

        /// vector of vertex positions (read only)
        virtual const std::vector<vec3>& points() const = 0;

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

        /**
         * The color scheme of a drawable.
         * Each drawable of a model can be colored in multiple ways, e.g.,
         *      - using a uniform color, by Drawable::set_default_color() and Drawable::set_per_vertex_color(false);
         *      - using one of the color properties (e.g., "v:color", "f:color", "e:color", or whatever);
         *      - textured using one of the texture coordinates (e.g., "v:texcoord", "h:texcoord", or whatever);
         *      - using scalar properties (e.g., "v:height", "f:area", or whatever), either textured or not.
         * @param The drawable to be queried.
         * @return The string denoting the current color scheme of the concerned drawable.
         */
        std::string color_scheme(const Drawable* d) const;
        void set_color_scheme(const Drawable* d, const std::string& scheme);

        /// prints the names of all properties
        virtual void property_stats() const = 0;

    protected:
        std::string	name_;
        bool		visible_;
        Box3		bbox_;
        bool		bbox_known_;

        std::vector<PointsDrawable*>    points_drawables_;
        std::vector<LinesDrawable*>     lines_drawables_;
        std::vector<TrianglesDrawable*> triangles_drawables_;

        // helper for rendering
        // A string denoting the current color scheme of a drawable.
        std::unordered_map<const Drawable*, std::string> color_scheme_;
    };



    inline std::string Model::color_scheme(const Drawable* d) const {
        auto pos = color_scheme_.find(d);
        if (pos == color_scheme_.end()) {
            auto model = const_cast<Model*>(this);
            model->set_color_scheme(d, "uniform color");
            return color_scheme(d);
        }
        else
            return pos->second;
    }

    inline void Model::set_color_scheme(const Drawable* d, const std::string& scheme) {
        color_scheme_[d] = scheme;
    }
}

#endif  // EASY3D_MODEL_H
