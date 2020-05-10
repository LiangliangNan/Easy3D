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

#ifndef EASY3D_RENDERER_GL_DRAWABLE_H
#define EASY3D_RENDERER_GL_DRAWABLE_H

#include <string>
#include <vector>
#include <functional>

#include <easy3d/core/types.h>
#include <easy3d/core/drawable.h>

namespace easy3d {

    class DrawableGL : public Drawable {
    public:
        // a drawable can be stand-alone or attached to a model
        DrawableGL(const std::string &name = "unknown", Model *model = nullptr);
        ~DrawableGL();

        // ------------------- buffer management ------------------------

        /**
         * Create/Update a single buffer.
         * Primitives like lines and triangles can be drawn with or without the element buffer.
         *  - With an element buffer: this can reduce the GPU memory consumption.
         *  - Without an element buffer: easier data transfer, but uses more GPU memory. In this case, vertices need to
         *    be in a correct order, like f1_v1, f1_v2, f1_v3, f2_v1, f2_v2, f2_v3... This requires the shared vertices
         *    be duplicated in the vertex buffer.
         */
        void update_vertex_buffer(const std::vector<vec3> &vertices);
        void update_color_buffer(const std::vector<vec3> &colors);
        void update_normal_buffer(const std::vector<vec3> &normals);
        void update_texcoord_buffer(const std::vector<vec2> &texcoords);
        void update_element_buffer(const std::vector<unsigned int> &elements);

        // entry must have 2 or 3 elements
        void update_element_buffer(const std::vector< std::vector<unsigned int> > &elements);

        /// selection buffer (internally based on a shader storage buffer)
        /// @param index: the index of the binding point.
        /// NOTE: the buffers should also be bound to this point in all shader code
        void update_selection_buffer(unsigned int index = 1);

        /// generic storage buffer
        /// @param index: the index of the binding point.
        /// NOTE: the buffers should also be bound to this point in all shader code
        void update_storage_buffer(const void *data, std::size_t datasize, unsigned int index = 0);

        /// Releases the element buffer if existing vertex data is sufficient (may require duplicating vertex data).
        void release_element_buffer();

        // ----------------- access data from the buffers -------------------

        void fetch_selection_buffer();

        // -------------------------- rendering -----------------------------

        /// The draw method.
        void draw(const Camera *camera, bool with_storage_buffer = false) const = 0;

        /// The internal draw method of this drawable.
        /// NOTE: this functions should be called when your shader program is in use,
        ///		 i.e., between glUseProgram(id) and glUseProgram(0);
        void gl_draw(bool with_storage_buffer = false) const;

    protected:
        void internal_update_buffers();
        void clear();
    };

}


#endif  // EASY3D_RENDERER_GL_DRAWABLE_H
