//
// Created by lnan on 23/01/2020.
//

#ifndef EASY3D_MANIFOLD_GUARD_H
#define EASY3D_MANIFOLD_GUARD_H

#include <easy3d/core/surface_mesh.h>


namespace easy3d {

    /**
     * A manifold guard that resolves non-manifoldness when constructing a surface mesh.
     */
    class ManifoldGuard {
    public:
        ManifoldGuard(SurfaceMesh* mesh) : mesh_(mesh) {}
        ~ManifoldGuard() {}

        void begin_surface();
        SurfaceMesh::Vertex add_vertex(const vec3& p);
        void begin_face();
        SurfaceMesh::Face end_face();   // returns the constructed face
        void end_suface();

    private:
        SurfaceMesh* mesh_;

    };

}

#endif //EASY3D_MANIFOLD_GUARD_H
