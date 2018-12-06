/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of EasyGUI: software for processing and rendering
*   meshes and point clouds.
*
*	EasyGUI is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	EasyGUI is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <easy3d/model/point_cloud.h>

using namespace easy3d;

int main(int /* argc */, char ** /* argv */) {
	// Create a point cloud
	Point_cloud cloud;

	// Now we add some points.
	// In this example, we add 8k points on a 20*20*20 grid.
	for (float i=-10; i<10; ++i) {
		for (float j = -10; j < 10; ++j) {
			for (float k = -10; k < 10; ++k) {
				cloud.add_vertex(vec3(i, j, k));
			}
		}
	}

	std::cout << "point cloud has " << cloud.n_vertices() << " points" << std::endl;

    return EXIT_SUCCESS;
}
