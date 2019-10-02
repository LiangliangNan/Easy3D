#include "point_set_qualities.h"
#include <geometry/point_set.h>
#include <kd_tree/kd_tree_search_nanoflann.h>
#include <math/principal_axes.h>
#include <basic/stop_watch.h>
#include <basic/logger.h>


void PointSetQualities::compupte_planarities(PointSet* pset, const std::vector<unsigned int>& neighbor_sizes)
{
	if (neighbor_sizes.empty()) {
		Logger::warn(title()) << "you need to specify at least one neighborhood size" << std::endl;
		return;
	}

	StopWatch t;

	Logger::out(title()) << "building kd-tree... " << std::endl;
	t.start();
	KdTreeSearch::Ptr kdtree = new KdTreeSearch_NanoFLANN;
	kdtree->begin();
	kdtree->add_point_set(pset);
	kdtree->end();
	Logger::out(title()) << "building kd-tree done. time: " << t.elapsed_seconds() << " seconds." << std::endl;

	Logger::out(title()) << "computing planar qualities... " << std::endl;
	t.restart();

	int num_scales = neighbor_sizes.size();

	std::vector<float>& planar_qualities = pset->vertex_property<float>("v:planarity").vector();
	std::vector<vec3>& points = pset->get_vertex_property<vec3>("v:point").vector();

#pragma omp parallel for
	for (int i = 0; i < points.size(); ++i) {
		const vec3& p = points[i];
		std::vector< std::vector<double> > eigen_values(num_scales);
		for (int i = 0; i < num_scales; ++i)
			eigen_values[i].resize(3);
		for (int j = 0; j < num_scales; ++j) {
			std::vector<int> indices;
			kdtree->find_closest_K_points(p, neighbor_sizes[j], indices);

			PrincipalAxes3 pca;
			pca.begin();
			for (unsigned int k = 0; k < indices.size(); ++k)
				pca.add_point(points[indices[k]]);
			pca.end();

			for (int k = 0; k < 3; ++k)
				eigen_values[j][k] = pca.eigen_value(3 - k - 1); // eigen values are sorted in descending order

			assert(eigen_values[j][0] <= eigen_values[j][1] && eigen_values[j][1] <= eigen_values[j][2]);
		}

		double conf = 0.0;
		for (int j = 0; j < num_scales; ++j) {
			conf += (1 - 3.0 * eigen_values[j][0] / (eigen_values[j][0] + eigen_values[j][1] + eigen_values[j][2])) * (eigen_values[j][1] / eigen_values[j][2]);
		}
		conf /= 3.0;
		planar_qualities[i] = static_cast<float>(conf);
	}

	Logger::out(title()) << "computing planar qualities done. time:" << t.elapsed_seconds() << " seconds." << std::endl;
}
