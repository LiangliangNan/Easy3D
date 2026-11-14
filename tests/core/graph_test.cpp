// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/core/graph.h>
#include <easy3d/fileio/graph_io.h>
#include <algorithm>
#include <memory>
#include <set>
#include <sstream>
using namespace easy3d;
namespace {
// A square with a diagonal and one vertex nobody connects to.
struct Fixture {
    Graph graph;Graph::Vertex a,b,c,d,lone;
    Fixture():a(graph.add_vertex(vec3(0))),b(graph.add_vertex(vec3(1,0,0))),c(graph.add_vertex(vec3(2,1,0))),
             d(graph.add_vertex(vec3(1,2,0))),lone(graph.add_vertex(vec3(9,9,9))) {
        graph.add_edge(a,b);graph.add_edge(b,c);graph.add_edge(c,d);graph.add_edge(d,a);graph.add_edge(a,c);
    }
};
std::vector<Graph::Edge> incident_edges(const Graph& g,Graph::Vertex v) {
    std::vector<Graph::Edge> found;for(auto e:g.edges(v)) found.push_back(e);return found;
}
// The live edges described by the positions of their endpoints: unaffected by the
// renumbering that collect_garbage() performs.
std::set<std::pair<int,int>> geometry_edges(const Graph& g) {
    const auto key=[](const vec3&p) {return int(p.x*100)+int(p.y);};
    std::set<std::pair<int,int>> edges;
    for(auto e:g.edges()) edges.insert(std::minmax(key(g.position(g.source(e))),key(g.position(g.target(e)))));
    return edges;
}
}
int main() { return test::run("graph connectivity, properties and I/O",[] {
    {   // the original file round trip
        Graph graph;auto a=graph.add_vertex(vec3(0)),b=graph.add_vertex(vec3(1,0,0)),c=graph.add_vertex(vec3(0,1,0));
        graph.add_edge(a,b);graph.add_edge(b,c);graph.add_edge(c,a);
        test::require(graph.n_vertices()==3 && graph.n_edges()==3,"graph counts differ");
        auto length=graph.add_edge_property<float>("e:length");
        for(auto e:graph.edges()) {length[e]=graph.edge_length(e);test::require(length[e]>0,"invalid edge length");}
        for(auto v:graph.vertices()) {size_t degree=0;for(auto other:graph.vertices(v)) {test::require(other!=v,"self edge created");++degree;}test::require(degree==2,"vertex adjacency differs");}
        test::require(GraphIO::save("graph.ply",&graph),"graph save failed");
        std::unique_ptr<Graph> loaded(GraphIO::load("graph.ply"));
        test::require(loaded && loaded->n_vertices()==3 && loaded->n_edges()==3,"graph round trip failed");
    }

    Fixture f;
    test::require(f.graph.n_vertices()==5 && f.graph.n_edges()==5,"the fixture is not what it should be");
    test::require(f.graph.add_edge(f.a,f.b)==Graph::Edge(0),"edges must be numbered in insertion order");
    test::require(f.graph.add_edge(f.a,f.b)==f.graph.find_edge(f.b,f.a) && f.graph.n_edges()==5,"a duplicated edge was created");
    test::require(!f.graph.add_edge(f.a,f.a).is_valid(),"a self-loop was accepted");
    test::require(!f.graph.add_edge(Graph::Vertex(),f.a).is_valid(),"an edge to an invalid vertex was accepted");
    test::require(!f.graph.find_edge(f.b,f.d).is_valid(),"a missing edge was found");
    test::require(!f.graph.find_edge(Graph::Vertex(),f.a).is_valid(),"an invalid vertex has no edges");

    // connectivity
    test::require(f.graph.valence(f.a)==3 && f.graph.valence(f.b)==2,"vertex valence differs");
    test::require(f.graph.is_isolated(f.lone) && !f.graph.is_isolated(f.c),"isolation is wrong");
    test::require(incident_edges(f.graph,f.a).size()==3 && incident_edges(f.graph,f.lone).empty(),"incident edges differ");
    for(auto e:f.graph.edges()) {
        test::require(f.graph.vertex(e,0)==f.graph.source(e) && f.graph.vertex(e,1)==f.graph.target(e),"edge endpoints disagree");
        test::require(f.graph.edge_length(e)>0,"invalid edge length");
    }
    {   // the circulators must visit every incident element once and wrap around
        auto it=f.graph.edges(f.a),start=it;
        size_t visited=0;
        if(it) do { ++visited; } while(++it != start);
        test::require(visited==f.graph.valence(f.a),"forward circulation did not visit each incident edge once");
        test::require(*--f.graph.edges(f.a)==incident_edges(f.graph,f.a).back(),"decrement did not wrap to the last incident edge");
        const auto list=incident_edges(f.graph,f.a);
        std::vector<Graph::Edge> forward,backward;
        { auto c=f.graph.edges(f.a); for(size_t i=0;i<list.size();++i) {forward.push_back(*c);++c;} }
        { auto c=f.graph.edges(f.a); for(size_t i=0;i<list.size();++i) {backward.push_back(*c);--c;} }
        test::require(forward==list,"forward circulation and the incident edges differ");
        test::require(backward[0]==list[0] && backward[1]==list[2] && backward[2]==list[1],"backward circulation does not visit the incident edges in reverse");
        size_t neighbors=0;
        for(auto v:f.graph.vertices(f.c)) {test::require(v!=f.c,"a vertex is its own neighbor");++neighbors;}
        test::require(neighbors==f.graph.valence(f.c),"neighbor iteration and valence differ");
    }

    // properties
    auto tag=f.graph.add_vertex_property<int>("v:tag");
    test::require(bool(tag),"the vertex property was not added");
    test::require(!f.graph.add_vertex_property<int>("v:tag"),"two vertex properties share a name");
    test::require(!bool(f.graph.get_vertex_property<float>("v:tag")),"a property of the wrong type was returned");
    test::require(f.graph.get_vertex_property_type("v:tag")==typeid(int),"unexpected vertex property type");
    auto temporary=f.graph.add_vertex_property<int>("v:temporary");
    test::require(f.graph.remove_vertex_property(temporary) && !bool(temporary),"temporary property removal failed");
    test::require(!f.graph.remove_vertex_property("v:necessary"),"removing an unknown property succeeded");
    const std::vector<vec3> positions={f.graph.position(f.a),f.graph.position(f.b),f.graph.position(f.c),f.graph.position(f.d),f.graph.position(f.lone)};
    for(auto v:f.graph.vertices()) tag[v]=v.idx();
    auto label=f.graph.add_model_property<std::string>("m:label","square");
    test::require(label[0]=="square","the model property was not stored");
    test::require(f.graph.rename_model_property("m:label","m:name"),"model property rename failed");
    test::require(f.graph.get_model_property<std::string>("m:name")[0]=="square","the renamed property is gone");
    test::require(f.graph.vertex_properties()==std::vector<std::string>({"v:connectivity","v:point","v:deleted","v:tag"}),"unexpected vertex properties");
    std::ostringstream stats;f.graph.property_stats(stats);
    size_t occurrences=0;
    for(std::size_t pos=stats.str().find("m:name");pos!=std::string::npos;pos=stats.str().find("m:name",pos+1)) ++occurrences;
    test::require(occurrences==1,"property_stats lists a model property more than once");
    test::require(f.graph.remove_model_property("m:name"),"model property removal failed");
    test::require(!bool(f.graph.get_model_property<std::string>("m:name")),"the removed property is still there");

    // deletion: the incident edges go with the vertex and nothing is reachable afterwards
    const auto diagonal=f.graph.find_edge(f.a,f.c);
    f.graph.delete_edge(diagonal);
    test::require(f.graph.n_edges()==4 && f.graph.valence(f.a)==2,"delete_edge left the adjacency behind");
    test::require(f.graph.is_deleted(diagonal) && f.graph.has_garbage(),"the deleted edge is not marked");
    test::require(!f.graph.find_edge(f.a,f.c).is_valid(),"a deleted edge is still found");
    f.graph.delete_edge(diagonal);
    test::require(f.graph.n_edges()==4,"deleting an edge twice counted twice");
    f.graph.delete_vertex(f.b);
    test::require(f.graph.n_vertices()==4 && f.graph.n_edges()==2,"delete_vertex kept the incident edges");
    test::require(f.graph.is_deleted(f.b) && f.graph.is_isolated(f.b),"the deleted vertex is still connected");
    test::require(!f.graph.find_edge(f.a,f.b).is_valid(),"an edge to a deleted vertex survived");
    f.graph.delete_vertex(f.b);
    test::require(f.graph.n_vertices()==4,"deleting a vertex twice counted twice");
    f.graph.delete_vertex(Graph::Vertex());
    test::require(f.graph.n_vertices()==4,"an invalid vertex was deleted");
    size_t live_edges=0;for(auto e:f.graph.edges()) ++live_edges;
    test::require(live_edges==f.graph.n_edges(),"the edge iterator visits deleted edges");
    size_t live_vertices=0;for(auto v:f.graph.vertices()) ++live_vertices;
    test::require(live_vertices==f.graph.n_vertices(),"the vertex iterator visits deleted vertices");

    // garbage collection: everything live moves forward and keeps what it refers to
    const auto before=geometry_edges(f.graph);
    f.graph.collect_garbage();
    test::require(!f.graph.has_garbage(),"the garbage state survived");
    test::require(f.graph.n_vertices()==f.graph.vertices_size() && f.graph.n_edges()==f.graph.edges_size(),"the counts were not compacted");
    test::require(f.graph.n_vertices()==4 && f.graph.n_edges()==2,"collect_garbage removed the wrong elements");
    test::require(before.size()==2 && geometry_edges(f.graph)==before,"collect_garbage changed the connectivity");
    for(auto v:f.graph.vertices()) {
        test::require(!f.graph.is_deleted(v),"a deleted vertex survived the collection");
        test::require(f.graph.position(v)==positions[size_t(tag[v])],"a vertex property lost its owner");
        test::require(incident_edges(f.graph,v).size()==f.graph.valence(v),"the adjacency and the valence differ");
    }
    for(auto e:f.graph.edges()) {
        test::require(f.graph.is_valid(f.graph.source(e)) && f.graph.is_valid(f.graph.target(e)),"an edge points at a removed vertex");
        test::require(f.graph.find_edge(f.graph.source(e),f.graph.target(e))==e,"the edge lookup disagrees with the connectivity");
    }
    f.graph.reserve(16,32);
    test::require(f.graph.n_vertices()==4 && f.graph.n_edges()==2,"reserve() changed the graph");

    // copying: the copy is independent and assign() keeps only the standard properties
    Graph copy(f.graph);
    test::require(copy.n_vertices()==f.graph.n_vertices() && copy.n_edges()==f.graph.n_edges(),"the copy differs");
    Graph::Vertex corner;
    for(auto v:copy.vertices()) if(copy.position(v)==vec3(1,2,0)) corner=v;
    test::require(corner.is_valid(),"the copy lost a position");
    const size_t expected_edges=copy.n_edges()-copy.valence(corner);
    copy.delete_vertex(corner);
    test::require(copy.n_vertices()==f.graph.n_vertices()-1 && copy.n_edges()==expected_edges,"deleting in the copy does not work");
    test::require(f.graph.n_vertices()==4 && f.graph.n_edges()==2,"deleting from the copy changed the original");
    Graph assigned;assigned.assign(copy);
    test::require(assigned.n_vertices()==copy.n_vertices() && assigned.n_edges()==copy.n_edges(),"assign() differs");
    test::require(assigned.position(*assigned.vertices_begin())==copy.position(*copy.vertices_begin()),"assign() lost the positions");
    test::require(!bool(assigned.get_vertex_property<int>("v:tag")),"assign() copied a custom property");

    f.graph.clear();
    test::require(f.graph.n_vertices()==0 && f.graph.n_edges()==0 && !f.graph.has_garbage(),"clear() left elements behind");
    test::require(bool(f.graph.get_vertex_property<vec3>("v:point")),"clear() removed the standard properties");
    test::require(!bool(f.graph.get_vertex_property<int>("v:tag")),"clear() kept a custom property");
    test::require(f.graph.add_edge(f.graph.add_vertex(vec3(0)),f.graph.add_vertex(vec3(1))).is_valid(),"a graph after clear() cannot be used");
}); }
