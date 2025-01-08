#include <easy3d/algo/polygon_partition.h>
#include <easy3d/core/vec.h>

#include <memory>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


void bind_easy3d_algo_polygon_partition(pybind11::module_& m)
{
	{ // easy3d::PolygonPartition file:easy3d/algo/polygon_partition.h line:44
		pybind11::class_<easy3d::PolygonPartition, std::shared_ptr<easy3d::PolygonPartition>> cl(m, "PolygonPartition", "Convex partition of polygons.\n \n\n The algorithm assumes simply polygons without self-intersections. For complex unknown structures, you\n      may need to use the CSG operators provided in easy3d/algo/tessellator.h to obtain simply polygons first.\n \n\n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::PolygonPartition(); } ) );
		cl.def_static("apply_OPT", (bool (*)(const class std::vector<class easy3d::Vec<2, float> > &, class std::vector<class std::vector<unsigned long> > &)) &easy3d::PolygonPartition::apply_OPT, "Optimal convex partition (in terms of number of resulting convex polygons) of a polygon into\n      convex polygons by using the Keil-Snoeyink algorithm.\n \n\n The optimal convex partition algorithm is described in\n      M. Keil, J. Snoeyink, \"On the time bound for convex decomposition of simple polygons\", 1998.\n      Time complexity: O(n^3), where n is the number of vertices.\n      Space complexity: O(n^3).\n \n\n An input polygon (without holes). Vertices have to be in the counter-clockwise order.\n \n\n Resulting list of convex polygons, represented by the vertex indices.\n \n\n true on success, false on failure.\n \n\n apply_HM, apply.\n\nC++: easy3d::PolygonPartition::apply_OPT(const class std::vector<class easy3d::Vec<2, float> > &, class std::vector<class std::vector<unsigned long> > &) --> bool", pybind11::arg("poly"), pybind11::arg("parts"));
		cl.def_static("apply_HM", (bool (*)(const class std::vector<class easy3d::Vec<2, float> > &, class std::vector<class std::vector<unsigned long> > &)) &easy3d::PolygonPartition::apply_HM, "Partition a polygon into convex polygons by using the Hertel-Mehlhorn algorithm.\n \n\n The algorithm gives at most four times the number of parts as the optimal algorithm.\n      However, in practice it works much better than that and often gives optimal partition.\n      Time complexity: O(n^2), where n is the number of vertices.\n      Space complexity: O(n).\n      See S. Hertel and K. Mehlhorn. Fast triangulation of simple polygons.\n          4th Internat. Conf. Found. Comput. Theory, volume 158 of Lecture Notes Comput. Sci.,\n          pages 207-218. Springer-Verlag, 1983.\n \n\n An input polygon (without holes). Vertices have to be in the counter-clockwise order.\n \n\n Resulting list of convex polygons, represented by the vertex indices.\n \n\n true on success, false on failure.\n \n\n apply_OPT, apply.\n\nC++: easy3d::PolygonPartition::apply_HM(const class std::vector<class easy3d::Vec<2, float> > &, class std::vector<class std::vector<unsigned long> > &) --> bool", pybind11::arg("poly"), pybind11::arg("parts"));
		cl.def_static("apply", (bool (*)(const class std::vector<class easy3d::Vec<2, float> > &, const class std::vector<class std::vector<unsigned long> > &, const class std::vector<class std::vector<unsigned long> > &, class std::vector<class std::vector<unsigned long> > &)) &easy3d::PolygonPartition::apply, "Convex partition of a general polygon with an arbitrary number of non-hole and hole contours.\n \n\n This function partitions a list of polygons into convex parts by using the Hertel-Mehlhorn\n      algorithm. The algorithm gives at most four times the number of parts as the optimal algorithm.\n      However, in practice it works much better than that and often gives optimal partition.\n      Time complexity: O(n^2), where n is the number of vertices.\n      Space complexity: O(n).\n      See S. Hertel and K. Mehlhorn. Fast triangulation of simple polygons.\n          4th Internat. Conf. Found. Comput. Theory, volume 158 of Lecture Notes Comput. Sci.,\n          pages 207-218. Springer-Verlag, 1983.\n \n\n A set of points.\n \n\n A set of non-hole polygons (each represented by the vertex indices). The vertices of all\n     non-hole polygons have to be in counter-clockwise order.\n \n\n A set of holes (represented by the vertex indices). The vertices of all hole polygons\n     have to be in clockwise order.\n \n\n Resulting list of convex polygons (represented by the vertex indices).\n \n\n true on success, false on failure.\n \n\n apply_OPT, apply_HM.\n\nC++: easy3d::PolygonPartition::apply(const class std::vector<class easy3d::Vec<2, float> > &, const class std::vector<class std::vector<unsigned long> > &, const class std::vector<class std::vector<unsigned long> > &, class std::vector<class std::vector<unsigned long> > &) --> bool", pybind11::arg("points"), pybind11::arg("polys"), pybind11::arg("holes"), pybind11::arg("parts"));
	}
}
