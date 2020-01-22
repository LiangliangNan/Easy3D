
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/logging.h>

#include <algorithm>


namespace easy3d {


	struct FacetKey {
		FacetKey(SurfaceMesh::Vertex a1, SurfaceMesh::Vertex a2, SurfaceMesh::Vertex a3) {
			v1 = a1;
			v2 = a2;
			v3 = a3;
		}

		bool operator<(const FacetKey &rhs) const {
			if (v1.idx() < rhs.v1.idx()) { return true; }
			if (v1.idx() > rhs.v1.idx()) { return false; }
			if (v2.idx() < rhs.v2.idx()) { return true; }
			if (v2.idx() > rhs.v2.idx()) { return false; }
			return (v3.idx() < rhs.v3.idx());
		}

		SurfaceMesh::Vertex v1;
		SurfaceMesh::Vertex v2;
		SurfaceMesh::Vertex v3;
	};

	static std::set<FacetKey> *all_facet_keys;

	//_________________________________________________________


	void SurfaceMeshBuilder::begin_surface() {
		transition(initial, surface);
		star_ = mesh_->vertex_property<Star>("star");
		vertices_ = mesh_->vertex_property<vec3>("v:point");
		nb_non_manifold_v_ = 0;
		nb_duplicate_e_ = 0;
		nb_isolated_v_ = 0;
		all_facet_keys = new std::set<FacetKey>;
	}

	void SurfaceMeshBuilder::end_surface() {
		delete all_facet_keys;
		all_facet_keys = nullptr;

		transition(surface, final);
		terminate_surface();
		vertex_.clear();
		mesh_->remove_vertex_property<Star>(star_);
		if (!quiet_ && (nb_non_manifold_v_ != 0)) {
			LOG(WARNING) << "Encountered " << nb_non_manifold_v_
				<< " non-manifold vertices (fixed)";
		}
		if (!quiet_ && (nb_duplicate_e_ != 0)) {
			LOG(WARNING) << "Encountered " << nb_duplicate_e_
				<< " duplicated edges (fixed)";
		}
		if (!quiet_ && (nb_isolated_v_ != 0)) {
			LOG(WARNING) << "Encountered " << nb_isolated_v_
				<< " isolated vertices (removed)";
		}
		if (!quiet_ && mesh_->faces_size() == 0) {
			LOG(WARNING) << "0 facets. Is it a point cloud?";
		}
	}

	void SurfaceMeshBuilder::reset() {
		transition(final, initial);
	}

	SurfaceMesh::Vertex SurfaceMeshBuilder::add_vertex(unsigned int id, const vec3 &p) {
		transition(surface, surface);
		return add_vertex_internal(id, p);
	}

	SurfaceMesh::Vertex SurfaceMeshBuilder::add_vertex(const vec3 &p) {
		transition(surface, surface);
		return add_vertex_internal(p);
	}

	SurfaceMesh::Vertex SurfaceMeshBuilder::add_vertex_internal(unsigned int id, const vec3 &p) {
		SurfaceMesh::Vertex new_v = mesh_->add_vertex(p);
		while (vertex_.size() <= id) {
			vertex_.push_back(SurfaceMesh::Vertex());
		}
		vertex_[id] = new_v;
		return new_v;
	}

	SurfaceMesh::Vertex SurfaceMeshBuilder::add_vertex_internal(const vec3 &p) {
		return add_vertex_internal(static_cast<unsigned int>(vertex_.size()), p);
	}

	void SurfaceMeshBuilder::begin_facet() {
		facet_vertex_.clear();
	}

	void SurfaceMeshBuilder::end_facet() {
		std::size_t nb_vertices = facet_vertex_.size();
		if (nb_vertices < 3) {
			if (!quiet_) {
				LOG(ERROR) << "Facet with less than 3 vertices, ignored";
			}
			return;
		}

		// Detect duplicated vertices
		{
			for (int i = 0; i < nb_vertices; i++) {
				for (int j = i + 1; j < nb_vertices; j++) {
					if (facet_vertex_[i] == facet_vertex_[j]) {
						if (!quiet_) {
							LOG(ERROR) << "Facet with duplicated vertices, ignored";
						}
						return;
					}
				}
			}
		}

		// Detect and remove non-manifold edges by duplicating
		//   the corresponding vertices.
		{
			for (int from = 0; from < nb_vertices; from++) {
				int to = ((from + 1) % nb_vertices);
				if (find_halfedge_between(facet_vertex_[from], facet_vertex_[to]).is_valid()) {
					nb_duplicate_e_++;
					facet_vertex_[from] = copy_vertex(facet_vertex_[from]);
					facet_vertex_[to] = copy_vertex(facet_vertex_[to]);
				}
			}
		}

 		current_facet_ = mesh_->add_face(facet_vertex_);
	}


	SurfaceMesh::Vertex SurfaceMeshBuilder::copy_vertex(SurfaceMesh::Vertex from) {
		// Note: tex coords are not copied, since
		//  an halfedge does not exist in the copy to
		//  store the texvertex pointer.
		LOG_FIRST_N(WARNING, 5) << "vertex attributes are not copied";
		SurfaceMesh::Vertex result = mesh_->add_vertex(vertices_[from]);
		return result;
	}

	void SurfaceMeshBuilder::add_vertex_to_facet(int i) {
		if (i < 0 || i >= int(vertex_.size())) {
			if (!quiet_) {
				LOG(ERROR) << "vertex index " << i << " out of range";
			}
			return;
		}
		facet_vertex_.push_back(vertex_[i]);
	}

	void SurfaceMeshBuilder::add_vertex_to_facet_internal(SurfaceMesh::Vertex v) {
		transition(facet, facet);

		if (first_vertex_in_facet_.is_valid()) {
			SurfaceMesh::Halfedge new_halfedge =
				new_halfedge_between(current_vertex_, v);

			if (first_halfedge_in_facet_.is_valid()) {
				mesh_->set_next_halfedge(current_halfedge_, new_halfedge);
			}
			else {
				first_halfedge_in_facet_ = new_halfedge;
				mesh_->set_halfedge(mesh_->face(first_halfedge_in_facet_), first_halfedge_in_facet_);
			}
			current_halfedge_ = new_halfedge;
		}
		else {
			first_vertex_in_facet_ = v;
		}
		current_vertex_ = v;
	}


	SurfaceMesh::Vertex SurfaceMeshBuilder::current_vertex() {
		return vertex_[vertex_.size() - 1];
	}

	SurfaceMesh::Vertex SurfaceMeshBuilder::vertex(int i) {
		return vertex_[i];
	}

	SurfaceMesh::Face SurfaceMeshBuilder::current_facet() {
		return current_facet_;
	}

	SurfaceMesh::Halfedge SurfaceMeshBuilder::new_halfedge_between(SurfaceMesh::Vertex from, SurfaceMesh::Vertex to) {
		CHECK_TRUE(from != to);

		// Non-manifold edges have been removed
		// by the higher level public functions.
		// Let us do a sanity check anyway ...
		CHECK_TRUE(!find_halfedge_between(from, to).is_valid());

		SurfaceMesh::Halfedge result = mesh_->new_edge(from, to);
		mesh_->set_face(result, current_facet_);
		mesh_->set_vertex(result, to);

		star_[from].push_back(result);
		mesh_->set_halfedge(to, mesh_->opposite_halfedge(result));

		return result;
	}

	SurfaceMesh::Halfedge SurfaceMeshBuilder::find_halfedge_between(SurfaceMesh::Vertex from, SurfaceMesh::Vertex to) {
		Star &star = star_[from];
		for (Star::iterator it = star.begin(); it != star.end(); it++) {
			SurfaceMesh::Halfedge cur = *it;
			if (mesh_->to_vertex(cur) == to) {
				return cur;
			}
		}
		return SurfaceMesh::Halfedge();
	}

	bool SurfaceMeshBuilder::vertex_is_manifold(SurfaceMesh::Vertex v) {

		if (!mesh_->halfedge(v).is_valid()) {
			// Isolated vertex, removed in subsequent steps
			return true;
		}

		// A vertex is manifold if the stored and the
		// computed stars match (the number of halfedges
		// are tested).
		// Note: this test is valid only if the borders
		// have been constructed.

		return (int(star_[v].size()) == mesh_->valence(v));
	}

	bool SurfaceMeshBuilder::split_non_manifold_vertex(SurfaceMesh::Vertex v) {
		if (vertex_is_manifold(v)) {
			return false;
		}

		std::set<SurfaceMesh::Halfedge> star;
		{
			for (unsigned int i = 0; i < star_[v].size(); i++) {
				star.insert(star_[v][i]);
			}
		}

		// For the first wedge, reuse the vertex
		disconnect_vertex(mesh_->opposite_halfedge(mesh_->halfedge(v)), v, star);

		// There should be other wedges (else the vertex
		// would have been manifold)
		CHECK_TRUE(!star.empty());

		{
			// Create the vertices for the remaining wedges.
			while (!star.empty()) {
				SurfaceMesh::Vertex new_v = copy_vertex(v);
				SurfaceMesh::Halfedge h = *(star.begin());
				disconnect_vertex(h, new_v, star);
			}
		}

		return true;
	}


	void SurfaceMeshBuilder::disconnect_vertex(
		SurfaceMesh::Halfedge start_in, SurfaceMesh::Vertex v, std::set<SurfaceMesh::Halfedge > &star
	) {

		SurfaceMesh::Halfedge start = start_in;

		star_[v].clear();

		//   Important note: in this class, all the Stars correspond to the
		// set of halfedges radiating FROM a vertex (i.e. h->vertex() != v
		// if h belongs to Star(v) ).

		CHECK_TRUE(star.find(start) != star.end());


		// Note: the following code needs a special handling
		// of borders, since borders are not correctly connected
		// in a non-manifold vertex (the standard next_around_vertex
		// iteration does not traverse the whole star)

		while (!mesh_->is_boundary(start)) {
			start = mesh_->opposite_halfedge(mesh_->prev_halfedge(start));
			if (start == start_in) {
				break;
			}
		}

		mesh_->set_halfedge(v, start);

		SurfaceMesh::Halfedge cur = start;
		mesh_->set_vertex(mesh_->opposite_halfedge(cur), v);
		star_[v].push_back(cur);
		std::set<SurfaceMesh::Halfedge>::iterator it = star.find(cur);
		CHECK_TRUE(it != star.end());
		star.erase(it);

		while (!mesh_->is_boundary(mesh_->opposite_halfedge(cur))) {
			cur = mesh_->next_halfedge(mesh_->opposite_halfedge(cur));
			if (cur == start) {
				break;
			}

			mesh_->set_vertex(mesh_->opposite_halfedge(cur), v);
			std::set<SurfaceMesh::Halfedge>::iterator it = star.find(cur);
			CHECK_TRUE(it != star.end());
			star.erase(it);
			star_[v].push_back(cur);
		}

		if (mesh_->is_boundary(start)) {
			mesh_->set_next_halfedge(mesh_->opposite_halfedge(cur), start);
		}
	}


	void SurfaceMeshBuilder::terminate_surface() {
		return;

		// Step 1 : create the border halfedges, and setup the 'opposite'
		//   and 'vertex' pointers.
		for (auto it : mesh_->halfedges())
		{
			if (!mesh_->opposite_halfedge(it).is_valid()) {
				int a = 1;
			    //SurfaceMesh::Halfedge h =  mesh_->new_edge();
			    //make_opposite(h, it); 
			    //set_halfedge_vertex(h, it->prev()->vertex());
			
			
			    //// Used later to fix non-manifold vertices .
			    //star_[it->vertex()].push_back(h);
			}
		}

		// Step 2 : setup the 'next' and 'prev' pointers of the border.
		//
		for (auto it : mesh_->halfedges())
		{
			if (!mesh_->face(it).is_valid()) {

				SurfaceMesh::Halfedge next = mesh_->opposite_halfedge(it);
				while (mesh_->face(next).is_valid()) {
					next = mesh_->opposite_halfedge(mesh_->prev_halfedge(next));
				}
				mesh_->set_next_halfedge(it, next);

				SurfaceMesh::Halfedge prev = mesh_->opposite_halfedge(it);
				while (mesh_->face(prev).is_valid()) {
					prev = mesh_->opposite_halfedge(mesh_->next_halfedge(prev));
				}

				mesh_->set_next_halfedge(prev, it);
			}
		}

		// Step 3 : fix non-manifold vertices

		for (unsigned int i = 0; i < vertex_.size(); i++) {
			if (!vertex_[i].is_valid()) {
				continue;
			}
			if (split_non_manifold_vertex(vertex_[i])) {
				nb_non_manifold_v_++;
			}
		}


		// Step 4 : create TexVertices

//         FOR_EACH_HALFEDGE(Mesh, target(), it)
//         {
//             if (it->tex_vertex() == nil) {
//                 TexVertex *new_tv = new_tex_vertex();
//                 Halfedge *cur = it;
//                 do {
//                     set_halfedge_tex_vertex(cur, new_tv);
//                     cur = cur->next()->opposite();
//                 } while (cur->tex_vertex() == nil);
//                 cur = it->opposite()->prev();
//                 while (it->tex_vertex() == nil) {
//                     set_halfedge_tex_vertex(cur, new_tv);
//                     cur = cur->opposite()->prev();
//                 }
//             }
//         }

		// Step 5 : check for isolated vertices
		for (unsigned int i = 0; i < vertex_.size(); i++) {
			if (!vertex_[i].is_valid()) {
				continue;
			}
			if (star_[vertex_[i]].size() == 0) {
				nb_isolated_v_++;
				mesh_->delete_vertex(vertex_[i]);
			}
		}

		mesh_->garbage_collection();
		// target()->assert_is_valid();
	}

	void SurfaceMeshBuilder::transition(state from, state to) {
		check_state(from);
		state_ = to;
	}

	void SurfaceMeshBuilder::check_state(state s) {
		if (state_ != s) {
			LOG(ERROR) << "invalid state: \'"
				<< state_to_string(state_)
				<< "\', expected \'"
				<< state_to_string(s)
				<< "\'";
			bool ok = false;
			CHECK_TRUE(ok);
		}
	}

	std::string SurfaceMeshBuilder::state_to_string(state s) {
		switch (s) {
		case initial:
			return "initial";
		case surface:
			return "surface";
		case facet:
			return "facet";
		case final:
			return "final";
		default:
			return "unknown";
		}
		return "unknown";
	}

	void SurfaceMeshBuilder::set_vertex(unsigned int v, const vec3 &p) {
		vertices_[vertex(v)] = p;
	}

	void SurfaceMeshBuilder::create_vertices(unsigned int nb_vertices, bool with_colors) {
		for (unsigned int i = 0; i < nb_vertices; i++) {
			add_vertex(vec3(0.0, 0.0, 0.0));
		}
	}

}
