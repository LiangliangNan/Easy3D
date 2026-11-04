// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <memory>
using namespace easy3d;
int main() { return test::run("surface mesh builder, topology, properties and I/O",[] {
    SurfaceMesh mesh;SurfaceMeshBuilder builder(&mesh);builder.begin_surface();
    auto a=builder.add_vertex(vec3(0)),b=builder.add_vertex(vec3(1,0,0)),c=builder.add_vertex(vec3(0,1,0)),d=builder.add_vertex(vec3(0,0,1));
    builder.add_triangle(a,b,d);builder.add_triangle(b,c,d);builder.add_triangle(c,a,d);builder.add_triangle(a,c,b);builder.end_surface();
    test::require(mesh.n_vertices()==4 && mesh.n_edges()==6 && mesh.n_faces()==4,"tetrahedron topology differs");
    mesh.update_face_normals();auto normals=mesh.get_face_property<vec3>("f:normal");
    for(auto f:mesh.faces()) {size_t n=0;for(auto v:mesh.vertices(f)) {test::require(!mesh.is_border(v),"closed tetrahedron has a border");++n;}test::require(n==3 && normals[f].length()>.99f,"invalid face/normal");}
    test::require(SurfaceMeshIO::save("mesh.off",&mesh),"surface mesh save failed");
    std::unique_ptr<SurfaceMesh> loaded(SurfaceMeshIO::load("mesh.off"));
    test::require(loaded && loaded->n_faces()==4 && loaded->n_edges()==6,"mesh round trip failed");
    mesh.delete_face(SurfaceMesh::Face(0));mesh.collect_garbage();test::require(mesh.n_faces()==3,"face deletion failed");
}); }
