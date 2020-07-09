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

#ifndef EASY3D_CORE_MODEL_H
#define EASY3D_CORE_MODEL_H


#include <string>
#include <vector>

#include <easy3d/core/types.h>


namespace easy3d {

    class Renderer;

    /**
     * Model is the base class of renderable 3D models, e.g., SurfaceMesh, PointCloud, Graph.
     * A Model can have multiple drawables, e.g., faces, vertices, edges.
     */
    class Model
    {
    public:
        Model(const std::string& name = "unknown");
        virtual ~Model();

        const std::string& name() const { return name_; }
        void set_name(const std::string& n) { name_ = n; }

        /** The bounding box of the model. */
        const Box3& bounding_box() const;

        void update_bounding_box();

        /** The vertices of the model. */
        virtual const std::vector<vec3>& points() const = 0;

        /** Test if the model is empty. */
        bool empty() const { return points().empty(); };

        /** prints the names of all properties to an output stream (e.g., std::cout). */
        virtual void property_stats(std::ostream &output) const = 0;

        /** Set/Get the renderer of this model. */
        void set_renderer(Renderer* r) { renderer_ = r; }
        inline Renderer* renderer() { return renderer_; }
        inline const Renderer* renderer() const { return renderer_; }

    protected:
        std::string	name_;

        Box3		bbox_;
        bool		bbox_known_;

        Renderer* renderer_;
    };
}

#endif  // EASY3D_CORE_MODEL_H
