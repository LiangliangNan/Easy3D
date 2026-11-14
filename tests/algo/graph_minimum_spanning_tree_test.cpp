// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/algo/graph_minimum_spanning_tree.h>
#include <easy3d/core/graph.h>
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <vector>
using namespace easy3d;
namespace {
const float none = std::numeric_limits<float>::infinity();

// A tree is only a tree if every vertex walks up to its own root without looping.
void require_reaches_root(const Graph& g,const std::vector<int>& predecessor,int v,int root) {
    const std::size_t n=predecessor.size();
    for(std::size_t steps=0;steps<=n;++steps) {
        if(v==root) return;
        const int parent=predecessor.at(size_t(v));
        test::require(parent>=0,"a covered vertex has no parent");
        test::require(g.find_edge(Graph::Vertex(parent),Graph::Vertex(v)).is_valid(),"the tree uses an edge the graph does not have");
        v=parent;
    }
    test::require(false,"the predecessor relation contains a cycle");
}

float tree_weight(const Graph& g,const std::vector<float>& weight,const std::vector<int>& predecessor) {
    float total=0;
    for(std::size_t v=0;v<predecessor.size();++v) {
        if(predecessor[v]<0) continue;
        const auto e=g.find_edge(Graph::Vertex(predecessor[v]),Graph::Vertex(int(v)));
        test::require(e.is_valid(),"a tree edge is missing from the graph");
        total+=weight[size_t(e.idx())];
    }
    return total;
}

// Reference implementation: the dense O(V^2) Prim, which has neither the heap nor the
// tie-breaking of the tested one, so only the total weight is comparable.
float dense_minimum_weight(const std::vector<std::vector<float>>& matrix,int root) {
    const int n=static_cast<int>(matrix.size());
    std::vector<float> best(static_cast<std::size_t>(n),none);
    std::vector<char> used(static_cast<std::size_t>(n),0);
    best[size_t(root)]=0;
    float total=0;
    for(int i=0;i<n;++i) {
        int v=-1;
        for(int k=0;k<n;++k)
            if(!used[size_t(k)] && (v<0 || best[size_t(k)]<best[size_t(v)])) v=k;
        test::require(v>=0 && best[size_t(v)]<none,"the reference graph is not connected");
        used[size_t(v)]=1;
        total+=best[size_t(v)];
        for(int k=0;k<n;++k) best[size_t(k)]=std::min(best[size_t(k)],matrix[size_t(v)][size_t(k)]);
    }
    return total;
}

// The square a-b-c-d with the diagonal a-c, its five edges numbered in this order.
Graph square(std::vector<float>& weight,const std::vector<float>& weights) {
    Graph g;
    const auto a=g.add_vertex(vec3(0)),b=g.add_vertex(vec3(1,0,0)),c=g.add_vertex(vec3(1,1,0)),d=g.add_vertex(vec3(0,1,0));
    const Graph::Vertex ends[][2]={{a,b},{b,c},{c,d},{d,a},{a,c}};
    weight.clear();
    for(std::size_t i=0;i<5;++i) {
        test::require(g.add_edge(ends[i][0],ends[i][1])==Graph::Edge(int(i)),"edges must be numbered in insertion order");
        weight.push_back(weights[i]);
    }
    return g;
}
}
int main() { return test::run("graph minimum spanning tree",[] {
    // 1. hand checkable: the two cheapest edges of the triangle a-b-c-a and the cheapest
    //    edge leaving it, i.e. 1 + 2 + 4 = 7; the diagonal (3) and d-a (8) must be skipped.
    std::vector<float> weight;
    const auto triangle=square(weight,{1,2,4,8,3});
    std::vector<int> predecessor;
    test::require(GraphMinimumSpanningTree::compute(&triangle,weight,{0},predecessor),"mst of a connected graph failed");
    test::require(tree_weight(triangle,weight,predecessor)==7,"wrong total weight");
    for(int v=1;v<4;++v) require_reaches_root(triangle,predecessor,v,0);
    // rooting elsewhere must not change the tree, only the direction of the parent links
    std::vector<int> other;
    test::require(GraphMinimumSpanningTree::compute(&triangle,weight,{3},other),"mst rooted elsewhere failed");
    test::require(tree_weight(triangle,weight,other)==7,"the tree depends on its root");
    for(int v=0;v<3;++v) require_reaches_root(triangle,other,v,3);
    test::require(predecessor!=other,"the parent links ignore the root");

    // 2. equal weights: the tree has to use exactly three of the five edges and must stay
    //    a tree; which three is not unique.
    const auto ties=square(weight,{1,1,1,1,1});
    test::require(GraphMinimumSpanningTree::compute(&ties,weight,{1},predecessor),"mst with tied weights failed");
    int parents=0;
    for(auto p:predecessor) if(p>=0) ++parents;
    test::require(parents==3 && tree_weight(ties,weight,predecessor)==3,"a tied tree is not minimal");
    for(int v=0;v<4;++v) if(v!=1) require_reaches_root(ties,predecessor,v,1);

    // 3. negative weights are ordinary weights: the most negative edges win.
    const auto negative=square(weight,{-5,-4,-3,-2,-1});
    test::require(GraphMinimumSpanningTree::compute(&negative,weight,{2},predecessor),"mst with negative weights failed");
    test::require(tree_weight(negative,weight,predecessor)==-12,"negative weights were not minimized");
    //    a negative edge must not pull a root into another tree: the forest has one root
    //    per entry of roots whatever the weights are.
    const auto two_roots=square(weight,{-10,-5,1,1,1});
    test::require(GraphMinimumSpanningTree::compute(&two_roots,weight,{0,2},predecessor),"a negative edge disturbed the roots");
    test::require(predecessor[0]==-1 && predecessor[2]==-1,"a root received a predecessor");
    for(int v=0;v<4;++v) if(predecessor[size_t(v)]>=0) require_reaches_root(two_roots,predecessor,v,0);

    // 4. a forest: a triangle, a pair and an isolated vertex, one root each. A single root
    //    leaves the component it cannot reach untouched.
    Graph forest;
    const auto a=forest.add_vertex(vec3(0)),b=forest.add_vertex(vec3(1,0,0)),c=forest.add_vertex(vec3(0,1,0));
    const auto d=forest.add_vertex(vec3(5)),e=forest.add_vertex(vec3(6,1,0)),isolated=forest.add_vertex(vec3(9,9,0));
    weight={1,2,3,4};
    test::require(forest.add_edge(a,b)==Graph::Edge(0) && forest.add_edge(b,c)==Graph::Edge(1) &&
                  forest.add_edge(c,a)==Graph::Edge(2) && forest.add_edge(d,e)==Graph::Edge(3),"the forest is misnumbered");
    test::require(forest.n_edges()==4 && forest.is_isolated(isolated),"the forest is not what it should be");
    const std::vector<int> all_roots={int(a.idx()),int(d.idx()),int(isolated.idx())};
    test::require(GraphMinimumSpanningTree::compute(&forest,weight,all_roots,predecessor),"the forest was not covered");
    int covered=0;
    for(auto p:predecessor) if(p>=0) ++covered;
    test::require(covered==3 && predecessor[size_t(isolated.idx())]==-1,"a component of one vertex is not a root");
    test::require(tree_weight(forest,weight,predecessor)==7,"wrong total weight for the forest");
    test::require(GraphMinimumSpanningTree::compute(&forest,weight,{int(a.idx())},predecessor),"a one root forest failed");
    test::require(predecessor[size_t(d.idx())]==-1 && predecessor[size_t(e.idx())]==-1,"a root reached another component");

    // 5. bad input is refused instead of quietly producing a partial answer.
    std::vector<int> ignored;
    test::require(!GraphMinimumSpanningTree::compute(nullptr,weight,{0},ignored),"a null graph was accepted");
    test::require(!GraphMinimumSpanningTree::compute(&triangle,{1,2,3,4},{0},ignored),"too few weights were accepted");
    test::require(!GraphMinimumSpanningTree::compute(&triangle,{1,2,3,4,std::numeric_limits<float>::quiet_NaN()},{0},ignored),"a NaN weight was accepted");
    test::require(!GraphMinimumSpanningTree::compute(&triangle,weight,{7},ignored),"an out of range root was accepted");
    test::require(!GraphMinimumSpanningTree::compute(&triangle,weight,{-1},ignored),"a negative root was accepted");

    // 6. cross-check the heap based Prim against the dense one on connected random graphs.
    for(int seed=0;seed<20;++seed) {
        std::srand(unsigned(seed));
        const int n=6+seed%7;
        Graph random;
        for(int v=0;v<n;++v) random.add_vertex(vec3(float(std::rand())/RAND_MAX,float(std::rand())/RAND_MAX,0));
        std::vector<std::vector<float>> matrix(static_cast<std::size_t>(n),std::vector<float>(static_cast<std::size_t>(n),none));
        weight.clear();
        const auto link=[&](int u,int v) {
            const float w=float(weight.size()+1)+float(std::rand()%100)/100;
            matrix[size_t(u)][size_t(v)]=matrix[size_t(v)][size_t(u)]=w;
            test::require(random.add_edge(Graph::Vertex(u),Graph::Vertex(v))==Graph::Edge(int(weight.size())),"the random graph is misnumbered");
            weight.push_back(w);
        };
        for(int v=1;v<n;++v) link(std::rand()%v,v); // a spanning tree: connected by construction
        for(int extra=n/2+std::rand()%n;extra>0;--extra) {
            const int u=std::rand()%n,v=std::rand()%n;
            if(u==v || matrix[size_t(u)][size_t(v)]<none) continue; // already connected
            link(u,v);
        }
        test::require(GraphMinimumSpanningTree::compute(&random,weight,{0},predecessor),"the random graph failed");
        test::require(tree_weight(random,weight,predecessor)==dense_minimum_weight(matrix,0),"not a minimum spanning tree");
        for(int v=1;v<n;++v) require_reaches_root(random,predecessor,v,0);
    }
}); }
