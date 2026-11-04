// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/core/graph.h>
#include <easy3d/fileio/graph_io.h>
#include <memory>
#include <cmath>
using namespace easy3d;
int main() { return test::run("graph connectivity, properties and I/O",[] {
    Graph graph;auto a=graph.add_vertex(vec3(0)),b=graph.add_vertex(vec3(1,0,0)),c=graph.add_vertex(vec3(0,1,0));
    graph.add_edge(a,b);graph.add_edge(b,c);graph.add_edge(c,a);
    test::require(graph.n_vertices()==3 && graph.n_edges()==3,"graph counts differ");
    auto length=graph.add_edge_property<float>("e:length");
    for(auto e:graph.edges()) {length[e]=graph.edge_length(e);test::require(length[e]>0,"invalid edge length");}
    for(auto v:graph.vertices()) {size_t degree=0;for(auto other:graph.vertices(v)) {test::require(other!=v,"self edge created");++degree;}test::require(degree==2,"vertex adjacency differs");}
    test::require(GraphIO::save("graph.ply",&graph),"graph save failed");
    std::unique_ptr<Graph> loaded(GraphIO::load("graph.ply"));
    test::require(loaded && loaded->n_vertices()==3 && loaded->n_edges()==3,"graph round trip failed");
}); }
