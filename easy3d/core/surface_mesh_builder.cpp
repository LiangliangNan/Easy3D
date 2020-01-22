
#include "mesh_builder.h"
#include "mesh.h"
#include <basic/logger.h>

#include <algorithm>


struct FacetKey {
	FacetKey(Mesh::Vertex* a1, Mesh::Vertex* a2, Mesh::Vertex* a3) {
		v1 = a1 ; v2 = a2 ; v3 = a3 ;
	}
	bool operator<(const FacetKey& rhs) const {
		if(v1 < rhs.v1) { return true ; }
		if(v1 > rhs.v1) { return false ; }
		if(v2 < rhs.v2) { return true ; }
		if(v2 > rhs.v2) { return false ; }
		return (v3 < rhs.v3) ; 
	}
	Mesh::Vertex* v1 ;
	Mesh::Vertex* v2 ;
	Mesh::Vertex* v3 ;
} ;

static std::set<FacetKey>* all_facet_keys ;

//_________________________________________________________


void MeshBuilder::begin_surface() {
	transition(initial, surface) ;
	star_.bind(target()) ;
	is_locked_.bind(target()) ;
	nb_non_manifold_v_ = 0 ;
	nb_duplicate_e_    = 0 ;
	nb_isolated_v_     = 0 ;
	all_facet_keys = new std::set<FacetKey> ;
}

void MeshBuilder::end_surface() {
	delete all_facet_keys ; 
	all_facet_keys = nil ;

	transition(surface, final) ;
	terminate_surface() ;
	vertex_.clear() ;
	tex_vertex_.clear() ;
	star_.unbind() ;
	if(color_.is_bound()) {
		color_.unbind() ;
	}
	if(!quiet_ && (nb_non_manifold_v_ != 0)) {
		Logger::warn("MeshBuilder") 
			<< "Encountered " << nb_non_manifold_v_
			<< " non-manifold vertices (fixed)" << std::endl ;
	}
	if(!quiet_ && (nb_duplicate_e_ != 0)) {
		Logger::warn("MeshBuilder") 
			<< "Encountered " << nb_duplicate_e_
			<< " duplicated edges (fixed)" << std::endl ;
	}
	if(!quiet_ && (nb_isolated_v_ != 0)) {
		Logger::warn("MeshBuilder") 
			<< "Encountered " << nb_isolated_v_
			<< " isolated vertices (removed)" << std::endl ;
	}
	if (!quiet_ && target()->size_of_facets() == 0) {
		Logger::warn("MeshBuilder") 
			<< "0 facets. Is it a point cloud?" << std::endl ;
	}
}

void MeshBuilder::reset() {
	transition(final, initial) ;
}

Mesh::Vertex* MeshBuilder::add_vertex(unsigned int id, const vec3& p) {
	transition(surface, surface) ;
	return add_vertex_internal(id, p) ;
}

Mesh::Vertex* MeshBuilder::add_vertex(const vec3& p) {
	transition(surface, surface) ;
	return add_vertex_internal(p) ;
}

void MeshBuilder::add_tex_vertex(const vec2& p) {
    add_tex_vertex(static_cast<unsigned int>(tex_vertex_.size()), p) ;
}

void MeshBuilder::add_tex_vertex(unsigned int id, const vec2& p) {
	transition(surface, surface) ;
	Mesh::TexVertex* new_tv = new_tex_vertex() ;
	new_tv->set_tex_coord(p) ;
	while(tex_vertex_.size() <= id) {
		tex_vertex_.push_back(nil) ;
	}
	tex_vertex_[id] = new_tv ;
}

Mesh::Vertex* MeshBuilder::add_vertex_internal(unsigned int id, const vec3& p) {
	Mesh::Vertex* new_v = new_vertex() ;
	new_v->set_point(p) ;
	while(vertex_.size() <= id) {
		vertex_.push_back(nil) ;
	}
	vertex_[id] = new_v ;
	return new_v;
}

Mesh::Vertex* MeshBuilder::add_vertex_internal(const vec3& p) {
    return add_vertex_internal(static_cast<unsigned int>(vertex_.size()), p) ;
}

void MeshBuilder::begin_facet() {
	facet_vertex_.clear() ;
	facet_tex_vertex_.clear() ;
}

void MeshBuilder::end_facet() {
	mpl_debug_assert(facet_vertex_.size() == facet_tex_vertex_.size()) ;
	std::size_t nb_vertices = facet_vertex_.size() ;

	if(nb_vertices < 3) {
		if(!quiet_) {
			Logger::err("MeshBuilder")
				<< "Facet with less than 3 vertices, ignored"
				<< std::endl ;
		}
		return ;
	}

	if(false && nb_vertices == 3) {
		std::vector<Mesh::Vertex*> W = facet_vertex_ ;
		std::sort(W.begin(), W.end()) ;
		FacetKey k(W[0], W[1], W[2]) ;
		if(all_facet_keys->find(k) != all_facet_keys->end()) {
			std::cerr << '.' << std::flush ;
			return ;
		}
		all_facet_keys->insert(k) ;
	}


	// Detect duplicated vertices
	{for(int i=0; i<nb_vertices; i++) {
		for(int j=i+1; j<nb_vertices; j++) {
			if(facet_vertex_[i] == facet_vertex_[j]) {
				if(!quiet_) {
					Logger::err("MeshBuilder") 
						<< "Facet with duplicated vertices, ignored"
						<< std::endl ;
				}
				return ;
			}
		}
	}}

	// Detect and remove non-manifold edges by duplicating
	//   the corresponding vertices.
	{for(int from=0; from<nb_vertices; from++) {
		int to=((from+1) % nb_vertices) ;
		if(find_halfedge_between(facet_vertex_[from], facet_vertex_[to]) != nil) 
		{
			nb_duplicate_e_++ ;
			facet_vertex_[from] = copy_vertex(facet_vertex_[from]) ;
			facet_vertex_[to] = copy_vertex(facet_vertex_[to]) ;
		}
	}}


	begin_facet_internal() ;
	{for(int i=0; i<nb_vertices; i++) {
		add_vertex_to_facet_internal(facet_vertex_[i]) ;
		if(facet_tex_vertex_[i] != nil) {
			set_corner_tex_vertex_internal(facet_tex_vertex_[i]) ;
		}
	}}
	end_facet_internal() ;
}


Mesh::Vertex* MeshBuilder::copy_vertex(Vertex* from) {
	// Note: tex coords are not copied, since 
	//  an halfedge does not exist in the copy to
	//  store the texvertex pointer.

	Vertex* result = new_vertex() ;
	result->set_point(from->point()) ;
	if(is_locked_[from]) {
		is_locked_[result] = true ;
	}
	return result ;
}

void MeshBuilder::add_vertex_to_facet(int i) {
	if(i < 0 || i >= int(vertex_.size())) {
		if(!quiet_) {
			Logger::err("MeshBuilder") << "vertex index " 
				<< i << " out of range"
				<< std::endl ;
		}
		return ;
	}
	facet_vertex_.push_back(vertex_[i]) ;
	facet_tex_vertex_.push_back(nil) ;
}

void MeshBuilder::set_corner_tex_vertex(int i) {
	if(i < 0 || i >= int(tex_vertex_.size())) {
		if(!quiet_) {
			//                Logger::err("MeshBuilder") << "texvertex index " 
			//                                          << i << " out of range"
			//                                          << std::endl ;
		}
		return ;
	}
	*(facet_tex_vertex_.rbegin()) = tex_vertex_[i] ;
}

void MeshBuilder::lock_vertex(int i) {
	if(i < 0 || i >= int(vertex_.size())) {
		if(!quiet_) {
			Logger::err("MeshBuilder") << "vertex index " 
				<< i << " out of range"
				<< std::endl ;
		}
		return ;
	}
	is_locked_[vertex(i)] = true ;
}

void MeshBuilder::unlock_vertex(int i) {
	if(i < 0 || i >= int(vertex_.size())) {
		if(!quiet_) {
			Logger::err("MeshBuilder") << "vertex index " 
				<< i << " out of range"
				<< std::endl ;
		}
		return ;
	}
	is_locked_[vertex(i)] = false ;
}

void MeshBuilder::begin_facet_internal() {
	transition(surface, facet) ;
	current_facet_ = new_facet() ;
	first_vertex_in_facet_ = nil ;
	current_vertex_ = nil ;
	first_halfedge_in_facet_ = nil ;
	current_halfedge_ = nil ;
	first_tex_vertex_in_facet_ = nil ;
}

void MeshBuilder::end_facet_internal() {
	transition(facet, surface) ;
	Mesh::Halfedge* h = 
		new_halfedge_between(current_vertex_, first_vertex_in_facet_) ;

	make_sequence(current_halfedge_, h) ;
	make_sequence(h, first_halfedge_in_facet_) ;
	if(first_tex_vertex_in_facet_ != nil) {
		set_halfedge_tex_vertex(h, first_tex_vertex_in_facet_) ; 
	}
}

void MeshBuilder::add_vertex_to_facet_internal(Vertex* v) {
	transition(facet, facet) ;

	if(first_vertex_in_facet_ == nil) {
		first_vertex_in_facet_ = v ;
	} else {
		Mesh::Halfedge* new_halfedge = 
			new_halfedge_between(current_vertex_, v) ;

		if(first_halfedge_in_facet_ == nil) {
			first_halfedge_in_facet_ = new_halfedge ;
			make_facet_key(first_halfedge_in_facet_) ;
		} else {
			make_sequence(current_halfedge_, new_halfedge) ;
		}
		current_halfedge_ = new_halfedge ;
	}
	current_vertex_ = v ;
}

void MeshBuilder::set_corner_tex_vertex_internal(TexVertex* tv) {
	transition(facet, facet) ;
	if(current_halfedge_ == nil) {
		first_tex_vertex_in_facet_ = tv ;
	} else {
		set_halfedge_tex_vertex(current_halfedge_, tv) ;
	}
}


Mesh::Vertex* MeshBuilder::current_vertex() {
	return vertex_[vertex_.size() - 1] ;
}

Mesh::Vertex* MeshBuilder::vertex(int i) {
	return vertex_[i] ;
}

Mesh::TexVertex* MeshBuilder::current_tex_vertex() {
	return tex_vertex_[tex_vertex_.size() - 1] ;
}

Mesh::TexVertex* MeshBuilder::tex_vertex(int i) {
	return tex_vertex_[i] ;
}

Mesh::Facet* MeshBuilder::current_facet() {
	return current_facet_ ;
}

Mesh::Halfedge* MeshBuilder::new_halfedge_between(
	Vertex* from, Vertex* to
	) {

		mpl_debug_assert(from != to) ;

		// Non-manifold edges have been removed
		// by the higher level public functions.
		// Let us do a sanity check anyway ...
		mpl_debug_assert(find_halfedge_between(from, to) == nil) ;

		Mesh::Halfedge* result = new_halfedge() ;
		set_halfedge_facet(result, current_facet_) ;
		set_halfedge_vertex(result, to) ;

		Mesh::Halfedge* opposite = find_halfedge_between(to, from) ;
		if(opposite != nil) {
			make_opposite(result, opposite) ;
		}

		star_[from].push_back(result) ;
		set_vertex_halfedge(to, result) ;

		return result ;
}

Mesh::Halfedge* MeshBuilder::find_halfedge_between(
	Vertex* from, Vertex* to
	) {
		Star& star = star_[from] ;
		for(Star::iterator it = star.begin(); it != star.end(); it++) {
			Mesh::Halfedge* cur = *it ;
			if(cur->vertex() == to) {
				return cur ;
			}
		}
		return nil ;
}

bool MeshBuilder::vertex_is_manifold(Vertex* v) {

	if(v->halfedge() == nil) {
		// Isolated vertex, removed in subsequent steps
		return true ;
	}

	// A vertex is manifold if the stored and the 
	// computed stars match (the number of halfedges
	// are tested).
	// Note: this test is valid only if the borders
	// have been constructed.

	return (int(star_[v].size()) == v->degree()) ;
}

bool MeshBuilder::split_non_manifold_vertex(Vertex* v) {
	if(vertex_is_manifold(v)) {
		return false ;
	}

	std::set<Mesh::Halfedge*> star ;
	{ for(unsigned int i=0; i<star_[v].size(); i++) {
		star.insert(star_[v][i]) ;
	}}

	// For the first wedge, reuse the vertex
	disconnect_vertex(
		v->halfedge()->opposite(), v, star
		) ;

	// There should be other wedges (else the vertex
	// would have been manifold)
	mpl_debug_assert(!star.empty()) ;

	{
		MeshVertexLock is_locked(target()) ;
		is_locked[v] = true ;
		// Create the vertices for the remaining wedges.
		while(!star.empty()) {
			Vertex* new_v = copy_vertex(v) ;
			is_locked[new_v] = true ;
			Mesh::Halfedge* h = *(star.begin()) ;
			disconnect_vertex(h, new_v, star) ;
		}
	}

	return true ;
}


void MeshBuilder::disconnect_vertex(
								   Mesh::Halfedge* start_in, Mesh::Vertex* v, std::set<Mesh::Halfedge*>& star
								   ) 
{

	Mesh::Halfedge* start = start_in ;

	star_[v].clear() ;

	//   Important note: in this class, all the Stars correspond to the
	// set of halfedges radiating FROM a vertex (i.e. h->vertex() != v
	// if h belongs to Star(v) ).

	mpl_debug_assert(star.find(start) != star.end()) ;


	//   Note: the following code needs a special handling
	// of borders, since borders are not correctly connected
	// in a non-manifold vertex (the standard next_around_vertex
	// iteration does not traverse the whole star)

	while(!start->is_border()) {
		start = start->prev()->opposite() ;
		if(start == start_in) {
			break ;
		}
	}
	set_vertex_halfedge(v,start->opposite()) ;

	Mesh::Halfedge* cur = start ;
	set_halfedge_vertex(cur->opposite(), v) ;
	star_[v].push_back(cur) ;
	std::set<Mesh::Halfedge*>::iterator it = star.find(cur) ;
	mpl_debug_assert(it != star.end()) ;
	star.erase(it) ;

	while(!cur->opposite()->is_border()) {
		cur = cur->opposite()->next() ;
		if(cur == start) {
			break ;
		}
		set_halfedge_vertex(cur->opposite(), v) ;
		std::set<Mesh::Halfedge*>::iterator it = star.find(cur) ;
		mpl_debug_assert(it != star.end()) ;
		star.erase(it) ;
		star_[v].push_back(cur) ;
	} 

	if(start->is_border()) {
		make_sequence(cur->opposite(),start) ;
	}
}


void MeshBuilder::terminate_surface() {

	// Step 1 : create the border halfedges, and setup the 'opposite'
	//   and 'vertex' pointers.
	FOR_EACH_HALFEDGE(Mesh, target(), it) {
		if(it->opposite() == nil) {
			Mesh::Halfedge* h = new_halfedge() ;
			make_opposite(h, it) ;
			set_halfedge_vertex(h, it->prev()->vertex()) ;

			// Initialize texture coordinates
			set_halfedge_tex_vertex(
				h, it->prev()->tex_vertex()
				) ;

			// Used later to fix non-manifold vertices.
			star_[ it->vertex() ].push_back(h) ;
		}
	}

	// Step 2 : setup the 'next' and 'prev' pointers of the border.
	//
	FOR_EACH_HALFEDGE(Mesh, target(), it) {
		if(it->facet() == nil) {

			Mesh::Halfedge* next = it->opposite() ;
			while(next->facet() != nil) {
				next = next->prev()->opposite() ;
			}
			set_halfedge_next(it, next) ;

			Mesh::Halfedge* prev = it->opposite() ;
			while(prev->facet() != nil) {
				prev = prev->next()->opposite() ;
			}
			set_halfedge_prev(it, prev) ;
		}
	}

	// Step 3 : fix non-manifold vertices

	for(unsigned int i=0; i<vertex_.size(); i++) {
		if(vertex_[i] == nil) {
			continue ;
		}
		if(split_non_manifold_vertex(vertex_[i])) {
			nb_non_manifold_v_++ ;
		}
	}


	// Step 4 : create TexVertices

	FOR_EACH_HALFEDGE(Mesh, target(), it) {
		if(it->tex_vertex() == nil) {
			TexVertex* new_tv = new_tex_vertex() ;
			Halfedge* cur = it ;
			do {
				set_halfedge_tex_vertex(cur, new_tv) ;
				cur = cur->next()->opposite() ;
			} while(cur->tex_vertex() == nil) ;
			cur = it->opposite()->prev() ;
			while(it->tex_vertex() == nil) {
				set_halfedge_tex_vertex(cur, new_tv) ;
				cur = cur->opposite()->prev() ;
			}
		}
	}

	// Step 5 : check for isolated vertices
	for(unsigned int i=0; i<vertex_.size(); i++) {
		if(vertex_[i] == nil) {
			continue ;
		}
		if(star_[vertex_[i]].size() == 0) {
			nb_isolated_v_++ ;
			delete_vertex(vertex_[i]) ;
		}
	}

	target()->assert_is_valid() ;
} 

void MeshBuilder::transition(state from, state to) {
	check_state(from) ;
	state_ = to ;
}

void MeshBuilder::check_state(state s) {
	if(state_ != s) {
		Logger::err("MeshBuilder") << "invalid state: \'" 
			<< state_to_string(state_)
			<< "\', expected \'"
			<< state_to_string(s) 
			<< "\'"
			<< std::endl ;
		bool ok = false ;
		mpl_debug_assert(ok) ;
	}
}

std::string MeshBuilder::state_to_string(state s) {
	switch(s) {
		case initial:
			return "initial" ;
		case surface:
			return "surface" ;
		case facet: 
			return "facet" ;
		case final:
			return "final" ;
		default:
			return "unknown" ;
	}
	return "unknown" ;
}

void MeshBuilder::set_vertex(unsigned int v, const vec3& p) {
	vertex(v)->set_point(p) ;
}

void MeshBuilder::set_vertex_color(unsigned int v, const vec3& c) {
	if(!color_.is_bound()) {
		color_.bind(target(), "color") ;
	}
	color_[vertex(v)] = c ;
}

void MeshBuilder::create_vertices(unsigned int nb_vertices, bool with_colors) {
	for (unsigned int i = 0; i < nb_vertices; i++) {
		add_vertex(vec3(0.0, 0.0, 0.0));
	}

	if(with_colors && !color_.is_bound()) {
		color_.bind(target(), "color") ;
	}
}
