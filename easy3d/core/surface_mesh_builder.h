
#ifndef _GEOMETRY_MESH_BUILDER_H_
#define _GEOMETRY_MESH_BUILDER_H_

#include "geometry_common.h"
#include "mesh.h"
#include "mesh_attributes.h"

#include <set>
#include <vector>


class GEOM_API MeshBuilder : public MeshMutator {
private:
	enum state {initial, surface, facet, final} ;

public:
	MeshBuilder(Mesh* target = nil, bool quiet = false) 
		: MeshMutator(target)
		, state_(initial)
		, current_facet_(nil)
		, current_vertex_(nil)
		, first_vertex_in_facet_(nil)
		, first_halfedge_in_facet_(nil)
		, current_halfedge_(nil)
		, first_tex_vertex_in_facet_(nil)
		, quiet_(quiet)
	{
	}

	virtual void begin_surface() ;
	virtual void end_surface() ;
	virtual void reset() ;

	virtual Mesh::Vertex* add_vertex(const vec3& p) ;
	virtual void add_tex_vertex(const vec2& p) ;

	virtual Mesh::Vertex* add_vertex(unsigned int id, const vec3& p) ;
	virtual void add_tex_vertex(unsigned int id, const vec2& p) ;

	virtual void begin_facet() ;
	virtual void end_facet() ;
	virtual void add_vertex_to_facet(int i) ;
	virtual void set_corner_tex_vertex(int i) ;

	virtual void lock_vertex(int i) ;
	virtual void unlock_vertex(int i) ;

	virtual void set_vertex(unsigned int v, const vec3& p) ;
	virtual void set_vertex_color(unsigned int v, const vec3& c) ;

	virtual void create_vertices(unsigned int nb_vertices, bool with_colors = false) ;

	Mesh::Vertex* current_vertex() ;
	Mesh::Vertex* vertex(int i) ;
	Mesh::Facet* current_facet() ;
	Mesh::TexVertex* current_tex_vertex() ;        
	Mesh::TexVertex* tex_vertex(int i) ;

	void set_quiet(bool quiet) { quiet_ = quiet ; }

protected:

	// Important note: in this class, all the Stars correspond to the
	// set of halfedges radiating FROM a vertex (i.e. h->vertex() != v
	// if h belongs to Star(v) ).

	Mesh::Vertex* add_vertex_internal(unsigned int id, const vec3& p) ;
	Mesh::Vertex* add_vertex_internal(const vec3& p) ;

	void begin_facet_internal() ;
	void end_facet_internal() ;
	void add_vertex_to_facet_internal(Vertex* v) ;
	void set_corner_tex_vertex_internal(TexVertex* tv) ;

	Vertex* copy_vertex(Vertex* from_vertex) ;

	std::vector<Vertex*>    facet_vertex_ ;
	std::vector<TexVertex*> facet_tex_vertex_ ;

private:

	Mesh::Halfedge* new_halfedge_between(Vertex* from, Vertex* to) ;
	Mesh::Halfedge* find_halfedge_between(Vertex* from, Vertex* to) ;

	/**
	* Checks whether a vertex is manifold, by
	* checking that the star_ attribute and the
	* computed star have the same number of Halfedges.
	* Note: this test is valid only if the borders
	* have been constructed.
	*/
	bool vertex_is_manifold(Vertex* v) ;

	/**
	* splits a non-manifold vertex into several vertices, as
	* needed. returns true if the vertex was non-manifold.
	*/
	bool split_non_manifold_vertex(Vertex* v) ;

	/**
	* used by split_non_manifold_vertex:
	* sets v's halfedge pointer,
	* sets the halfedge vertex pointers to v, 
	* removes the concerned halfedges from star,
	* updates the star of the concerned vertex,
	* if borders are found, connects them
	*/
	void disconnect_vertex(
		Mesh::Halfedge* start, Mesh::Vertex* v, 
		std::set<Mesh::Halfedge*>& star
		) ;

	void terminate_surface() ;
	friend class MeshSerializer_eobj ;

	void transition(state from, state to) ;
	void check_state(state s) ;
	std::string state_to_string(state s) ;

	state state_ ;
	std::vector<Mesh::Vertex*> vertex_ ;
	std::vector<MeshTypes::TexVertex::Ptr> tex_vertex_ ;

	Mesh::Facet* current_facet_ ;
	Vertex*		current_vertex_ ;
	Vertex*		first_vertex_in_facet_ ;
	Mesh::Halfedge*	first_halfedge_in_facet_ ;
	Mesh::Halfedge*	current_halfedge_ ;
	Mesh::TexVertex* first_tex_vertex_in_facet_ ;

	typedef std::vector<Mesh::Halfedge*> Star ;
	MeshVertexAttribute<Star>	star_ ;
	MeshVertexAttribute<vec3>	color_ ;
	MeshVertexLock				is_locked_ ;

	int		nb_isolated_v_ ;
	int		nb_non_manifold_v_ ;
	int		nb_duplicate_e_ ;
	bool	quiet_ ;
} ;


#endif // _MESH_BUILDER_

