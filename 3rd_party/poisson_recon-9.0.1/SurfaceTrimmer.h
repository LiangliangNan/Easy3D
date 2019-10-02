#ifndef SURFACE_TRIMMER_H
#define SURFACE_TRIMMER_H

#include "Geometry.h"

template<class Vertex >
void trim_mesh(
	std::vector< Vertex >& vertices,
          std::vector< std::vector<int> >& polygons,
		  float trim_value,
		  float area_ratio,
		  bool triangulate,
		  int smooth_iteration = 0
		  );


#include "SurfaceTrimmer.inl"
#endif
