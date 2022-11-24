/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
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
        triangle_in_->numberofpoints = static_cast<int>(nb_vertices);

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