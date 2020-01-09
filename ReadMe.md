<img src="resources/images/logo.jpg" width="400">

### Easy3D is an open source library for 3D modeling, geometry processing, and rendering. It is implemented in C++ and designed with an emphasis on simplicity and efficiency. 
### Easy3D is intended for research and educational purposes, but it is also a good starting point for developing sophisticated 3D applications.


<center>
	
<img src="resources/images/cloud.jpg" height="180">  |  <img src="resources/images/mesh.jpg" height="180">  |  <img src="resources/images/scalar.jpg" height="180">
:-----------------------------------------:|:------------------------------------------:|:-------------------------------------------:
Fig.1 Point cloud                          |  Fig.2 Mesh                                |  Fig.3 Scalar field

<img src="resources/images/vector.jpg" height="180"> |   <img src="resources/images/scene.jpg" height="180">
:-----------------------------------------:|:------------------------------------------:
Vector field                               |   Rendering a model given camera intrinsic and extrinsic parameters


<img src="resources/images/edl.jpg" height="180"> | <img src="resources/images/shadow.jpg" height="180"> |  <img src="resources/images/transparency.jpg" height="180">
:--------------------------------------:|:------------------------------------------:|:------------------------------------------------:
Eye-dome lighting                       |  Shadow                                    |   Transparency	

<img src="resources/images/imposters.jpg" height="180">| <img src="resources/images/reconstruction.jpg" height="180"> |  <img src="resources/images/planes.jpg" height="180">
:-------------------------------------------:|:--------------------------------------------------:|:------------------------------------------------:
Point/Line imposters                         |  Surface reconstruction                            |   Plane extraction                          	    

</center>

  
### Key features ###
* Efficient data structures for representing and managing 3D models (i.e., point clouds, meshes, and graphs). Easy to add/access arbitrary types of per-element properties.
* High-level encapsulation of OpenGL and GLSL for convenient and efficient rendering (based on modern and faster programmable-shader-style rendering, i.e., no fixed function calls). Users do not need to touch the low-level APIs of OpenGL. 
* A bunch of rendering techniques, e.g., point/line imposters, ambient occlusion (SSAO), hard shadow (shadow maps), soft shadow (PCSS), eye-dome lighting (for rendering point clouds without normal information), transparency (average color blending, dual depth peeling), and more ...
* A set of widely used algorithms, e.g., point cloud normal estimation/re-orientation, [Poisson Surface Reconstruction](http://www.cs.jhu.edu/~misha/MyPapers/ToG13.pdf), [RANSAC](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.481.1514&rep=rep1&type=pdf).
* A viewer that can be used directly to visualize 3D scenes in various formats, which can also be easily extended.
* Step-by-step tutorials to get acquainted with the data structures, rendering techniques, and processing algorithms for 3D modeling and geometry processing. 
 
 
### A quick glance ###

Any types of 3D drawables (e.g., points, lines, triangles, and thus point clouds, mesh surfaces, scalar fields, vector fields) can be rendered by writing a few lines of code. For example, only two lines of code to have a viewer to visualize you meshes or point clouds

```c++
        Viewer viewer("MyViewer");
        viewer.run();
```

Of course, you can customize the drawables. For example, the following code renders a point cloud as a set of spheres

```c++
	// assume your point cloud has been loaded to the viewer
        PointsDrawable* drawable = cloud->points_drawable("vertices");
	drawable->set_impostor_type(PointsDrawable::SPHERE); // draw points as spheres.
	drawable->set_point_size(3.0f);	// set point size
```
or as a set of surfels (i.e., 3D discs)
```c++ 
	drawable->set_impostor_type(PointsDrawable::SURFEL);
``` 

By abstracting geometric elements as one of the above drawables, more general visualization can be done very conveniently. Figure 3 shows the visualization of a scalar field (i.e., height) defined on the mesh vertices.

### Build

Easy3D has been tested on macOS, Linux, and Windows. You should also be able to build it on other platforms.

Run CMake to generate project files for you IDE. In case you use *Qt Creator* or other IDEs that can handle the CMakeLists.txt file, simply open that file and then all files will be well organized as projects.

### License
Easy3D is free software; you can redistribute it and/or modify it under the terms of the 
GNU General Public License as published by the Free Software Foundation; either version 3
of the License or (at your option) any later version. The full text of the license can be
found in the accompanying 'License' file.

### Citation
If Easy3D is useful in your research/work, I would be grateful if you show your appreciation by citing it:

```bibtex
@misc{easy3d2018nan,
  title = {Easy3D: a lightweight, easy-to-use, and efficient C++ library for processing and rendering 3D data},
  author = {Liangliang, Nan},
  note = {\url{https://github.com/LiangliangNan/Easy3D}},
  year = {2018},
}
```
---

Should you have any questions, comments, or suggestions, please contact me at liangliang.nan@gmail.com

**_Liangliang Nan_**

https://3d.bk.tudelft.nl/liangliang/

Dec. 8, 2018

Copyright (C) 2018
