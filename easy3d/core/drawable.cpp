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

#include <easy3d/core/drawable.h>

#include <cassert>

#include <easy3d/core/model.h>


namespace easy3d {


    Drawable::Drawable(const std::string &name, Model *model)
            : name_(name), model_(model), vao_(nullptr), num_vertices_(0), num_indices_(0),
              update_needed_(false), update_func_(nullptr), vertex_buffer_(0), color_buffer_(0), normal_buffer_(0),
              texcoord_buffer_(0), element_buffer_(0), storage_buffer_(0), current_storage_buffer_size_(0),
              selection_buffer_(0), current_selection_buffer_size_(0) {
    }


    const Box3 &Drawable::bounding_box() const {
        if (model_)
            return model_->bounding_box();
        else
            return bbox_;
    }


    void Drawable::buffer_stats(std::ostream &output) const {
        std::cout << "\t" << name() << std::endl;

        if (vertex_buffer()) {
            output << "\t\tVertex buffer:     " << num_vertices_ << " vertices, "
                   << num_vertices_ * sizeof(vec3) << " bytes" << std::endl;
        }
        if (normal_buffer()) {
            output << "\t\tNormal buffer:     " << num_vertices_ << " normals, "
                   << num_vertices_ * sizeof(vec3) << " bytes" << std::endl;
        }
        if (color_buffer()) {
            output << "\t\tColor buffer:      " << num_vertices_ << " colors, "
                   << num_vertices_ * sizeof(vec3) << " bytes" << std::endl;
        }
        if (texcoord_buffer()) {
            output << "\t\tTexcoord buffer:   " << num_vertices_ << " texcoords, "
                   << num_vertices_ * sizeof(vec2) << " bytes" << std::endl;
        }
        if (element_buffer()) {
            output << "\t\tIndex buffer:      " << num_indices_ << " indices, "
                   << num_indices_ * sizeof(unsigned int) << " bytes" << std::endl;
        }
    }


    void Drawable::update() {
        bbox_.clear();
        update_needed_ = true;
    }

}