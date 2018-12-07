<img src="logo.png" width="400">

### Easy3D is a C++ library for non-expert users to quickly get experiences in 3D modeling and geometry processing. It is intended for research and educational purposes. But it is also a good starting point for developing sophisticated 3D applications.

---

### Key Features ###
* Efficient data structures for representing and managing3D models (i.e., point 
  clouds and meshes).
* High-level encapsulation of OpenGL and GLSL for convenient and efficient 
  rendering. Users do not need to touch the low-level OpenGL APIs.
* File IO of a few standard 3D formats (e.g., PLY, OBJ, OFF, XYZ).
* A viewer that can be used directly to visualized 3D scenes in various formats.
* Step-by-step tutorials to get acquainted with the data structures and basic rendering 
  for 3D modeling and data processing. 
 
---

### Code Example ###

Any types of 3D drawables (e.g., points, lines, triangles, and thus point 
clouds, mesh surfaces, scalar fields, vector fields) can be rendered by 
writing a few lines of code. The following example shows how to
* load a point cloud from a file
* create a drawable for rendering the point cloud
* use the viewer to visualize the point cloud

```c++
#include <easy3d/core/viewer.h>
#include <easy3d/model/point_cloud.h>
#include <easy3d/core/drawable.h>

using namespace easy3d;

void main() {
	// create the default Easy3D viewer
	Viewer viewer;

	// create a point cloud
	Point_cloud* cloud = new Point_cloud;

	// load point cloud data from a file
	cloud->read("../../../data/bunny.bin");
	// give the model a name (optional)
	cloud->set_name("bunny");

	// create a drawable (a set of points that can be rendered)
	PointsDrawable* drawable = cloud->add_point_drawable("points");

	// collect points, colors, and normals (if exist) and transfer them to GPU
	auto points = cloud->get_vertex_property<vec3>("v:point");
	drawable->update_vertex_buffer(points.vector());
	auto normals = cloud->get_vertex_property<vec3>("v:normal");
	if (normals)	// if normals exist
		drawable->update_normal_buffer(normals.vector());
	auto colors = cloud->get_vertex_property<vec3>("v:color");
	if (colors)	// if colors exist
		drawable->update_color_buffer(colors.vector());

	// add the model to the viewer
	viewer.add_model(cloud);

	// run the viewer
	viewer.run();
}
```

---
  
### License
Easy3D is free software; you can redistribute it and/or modify it under the terms of the 
GNU General Public License as published by the Free Software Foundation; either version 3
of the License or (at your option) any later version. The full text of the license can be
found in the accompanying LICENSE file.

---

Should you have any questions, comments, or suggestions, please contact me at: 
liangliang.nan@gmail.com

**_Liangliang Nan_**

https://3d.bk.tudelft.nl/liangliang/

Dec. 8, 2018

Copyright (C) 2018
