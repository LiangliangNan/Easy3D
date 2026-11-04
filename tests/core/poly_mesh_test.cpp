// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/fileio/poly_mesh_io.h>
#include <memory>
using namespace easy3d;
int main() { return test::run("polyhedral mesh cells, incidence and I/O",[] {
    PolyMesh mesh;auto a=mesh.add_vertex(vec3(0)),b=mesh.add_vertex(vec3(1,0,0)),c=mesh.add_vertex(vec3(0,1,0)),d=mesh.add_vertex(vec3(0,0,1));
    mesh.add_tetra(a,b,c,d);
    test::require(mesh.n_vertices()==4 && mesh.n_edges()==6 && mesh.n_faces()==4 && mesh.n_cells()==1,"tetrahedron counts differ");
    for(auto cell:mesh.cells()) {size_t n=0;for(auto v:mesh.vertices(cell)) {test::require(mesh.is_valid(v),"invalid incident vertex");++n;}test::require(n==4,"cell incidence differs");}
    auto labels=mesh.add_cell_property<int>("c:label",7);for(auto cell:mesh.cells()) test::require(labels[cell]==7,"cell property default differs");
    test::require(PolyMeshIO::save("tetra.plm",&mesh),"polyhedral mesh save failed");
    std::unique_ptr<PolyMesh> loaded(PolyMeshIO::load("tetra.plm"));
    test::require(loaded && loaded->n_cells()==1 && loaded->n_faces()==4,"polyhedral mesh round trip failed");
}); }
