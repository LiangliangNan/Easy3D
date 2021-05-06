/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
 ********************************************************************/

#include <easy3d/algo/delaunay_2d.h>

#include <cstring> // for memset

#include <easy3d/util/stop_watch.h>

extern "C" {
#include <3rd_party/triangle/triangle.h>
}


namespace easy3d {


    static void init_triangulateio(struct triangulateio *tri) {
        memset(tri, 0, sizeof(struct triangulateio));
    }

    static void free_triangulateio(struct triangulateio *tri) {
        free(tri->pointlist);
        free(tri->pointattributelist);
        free(tri->pointmarkerlist);
        free(tri->trianglelist);
        free(tri->triangleattributelist);
        free(tri->trianglearealist);
        free(tri->neighborlist);
        free(tri->segmentlist);
        free(tri->segmentmarkerlist);
        free(tri->holelist);
        free(tri->regionlist);
        free(tri->edgelist);
        free(tri->edgemarkerlist);
        free(tri->normlist);
        memset(tri, 0, sizeof(struct triangulateio));
    }

    Delaunay2::Delaunay2() : Delaunay(2) {
        triangle_in_ = new struct triangulateio;
        triangle_out_ = new struct triangulateio;

        init_triangulateio(triangle_in_);
        init_triangulateio(triangle_out_);
    }

    Delaunay2::~Delaunay2() {
        free_triangulateio(triangle_in_);
        free_triangulateio(triangle_out_);
        delete triangle_in_;
        delete triangle_out_;
    }

    void Delaunay2::set_vertices(unsigned int nb_vertices, const float *vertices) {
        LOG(INFO) << "Delaunay triangulation...";
        StopWatch t;

        Delaunay::set_vertices(nb_vertices, vertices);
        //free_triangulateio(triangle_out_) ;
        triangle_in_->numberofpoints = nb_vertices;

        // Liangliang: if you provide "vertices" as double type, you can just assign the pointer, but it
        //			   seems float may not work properly for some of my data, so I always use double.
        //triangle_in_->pointlist = const_cast<float*>(vertices) ;
        triangle_in_->pointlist = new double[triangle_in_->numberofpoints * 2];
        for (unsigned int i = 0; i < nb_vertices * 2; ++i) {
            triangle_in_->pointlist[i] = vertices[i];
        }

        // Q: quiet
        // z: numbering starts from 0
        // n: output neighbors
        //std::cerr << "Calling triangulate" << std::endl ;
        triangulate((char *) "Qzn", triangle_in_, triangle_out_, nullptr);

        //std::cerr << "Calling set_arrays" << std::endl ;
        set_arrays(
                triangle_out_->numberoftriangles,
                triangle_out_->trianglelist, triangle_out_->neighborlist
        );

        LOG(INFO) << "done. time: " << t.time_string();
    }

}