<p align="right">
    <b> <img src="https://img.shields.io/badge/platforms-Windows%20%7C%20macOS%20%7C%20Linux-green" title="Supported Platforms"/> </b> <br>
    <b> <img src="https://img.shields.io/badge/license-GPL-blue" title="license-GPL"/> </b> <br>
</p>


![3D model generated and rendered by Easy3D](resources/images/logo.jpg)

#### For 3D modeling, geometry processing, and rendering, designed with emphasis on simplicity and efficiency. 
Easy3D is implemented in C++ and comes with **Python bindings**. It is intended for research and educational purposes, but it is also a good starting point for developing sophisticated 3D applications. 
Compared to existing geometry processing libraries (such as [PMP](http://www.pmp-library.org/) and [libigl](https://github.com/libigl/libigl/)) that focus on the algorithmic aspect, Easy3D also provides a wider range of functionalities for user interactions and rendering. It supports two rendering backends — native **Vulkan** (the default) and **OpenGL** — selected at build time with a single CMake option, with the same client code building against either backend.

<p align="center"> 
     <img src="./resources/images/overview.jpg" width="600"> 
</p>

### Key features
* Efficient data structures for representing and managing 3D models such as point clouds and polygonal surfaces 
  (e.g., triangle meshes), polyhedral volumes (e.g., tetrahedral meshes), and graphs. 
  Easy to add/access arbitrary types of per-element properties. Non-manifoldness is automatically resolved when 
  loading models from files ...
  
* A set of widely used algorithms, e.g., point cloud normal estimation/re-orientation, Poisson surface reconstruction, 
  RANSAC, mesh simplification, subdivision, smoothing, parameterization, remeshing, and more (the implementation of
  several surface mesh processing algorithms was taken from [PMP](http://www.pmp-library.org/)).
  
* High-level encapsulation of the graphics APIs for convenient and efficient rendering — client code does not need to 
  touch the low-level APIs of Vulkan or OpenGL. A bunch of rendering techniques, e.g., point/line impostors, ambient 
  occlusion (GTAO), hard shadows (shadow maps), soft shadows (PCSS), eye-dome lighting (for rendering point clouds 
  without normal information), transparency (sorted alpha blending, average color blending, dual depth peeling, 
  per-pixel linked lists), and more — all available on both rendering backends.
  
* A viewer that can be used directly to visualize 3D scenes in various formats, which can also be easily extended. 
  Window and event handling are based on [GLFW](https://www.glfw.org/) (e.g., [the default viewer](https://github.com/LiangliangNan/Easy3D/blob/main/easy3d/viewer)), 
  with an optional [ImGui](https://github.com/ocornut/imgui) interface (see [the ImGui viewer tutorial](https://github.com/LiangliangNan/Easy3D/tree/main/tutorials/Tutorial_202_Viewer_imgui)).
  There is also <b>Mapple</b>, a handy tool created out of the Easy3D library for rendering and processing 3D data 
  (built on the OpenGL backend).
  
* Python bindings for Easy3D, which allow you to use Easy3D in Python scripts (with either rendering backend). 
  
* Very easy to use as a callable library (usually only a few lines of code), with 60+ step-by-step 
  [C++ tutorials](https://liangliangnan.github.io/Easy3D/examples.html) demonstrating various uses of the API, to get 
  acquainted with the data structures, rendering techniques, user interaction, and algorithms for 3D modeling and 
  geometry processing (with detailed explanations in code), and a series of 
  [Python tutorials](https://liangliangnan.github.io/Easy3D/python/tutorials/) covering data structures, rendering, 
  and geometry processing in Python.

|              Scalar field                |              Polyhedral mesh              |              Keyframe animation            |
|------------------------------------------|-------------------------------------------|--------------------------------------------|
| ![](resources/images/mapple-scalar.jpg)  | ![](resources/images/mapple-polymesh.gif) | ![](resources/images/mapple-animation.gif) |

### A glance

Any type of 3D drawables (e.g., points, lines, triangles, and thus point clouds, mesh surfaces, scalar fields, 
and vector fields) can be rendered by writing a few lines of code with Easy3D. For example, the following code renders a 
point cloud as a set of spheres
```c++
// assume your point cloud has been loaded to the viewer
PointsDrawable* drawable = cloud->renderer()->get_points_drawable("vertices");
drawable->set_impostor_type(PointsDrawable::SPHERE); // draw points as spheres.
drawable->set_point_size(3.0f);    // set point size
```
or as a set of surfels (i.e., 3D discs)

```c++
drawable->set_impostor_type(PointsDrawable::SURFEL);
``` 

By abstracting geometric elements as one of the above drawables, more general visualization (e.g., vector fields, 
scalar fields) can be done very conveniently.

### Easy3D repository layout
The repository contains a `CMakeLists.txt` file (in the root directory of the repository) that serves as an anchor for 
configuring and building programs, as well as a set of subfolders:
* [`3rd_party`](https://github.com/LiangliangNan/Easy3D/tree/main/3rd_party) - source code of third-party libraries
* [`applications`](https://github.com/LiangliangNan/Easy3D/tree/main/applications) - applications built on top of Easy3D
* [`cmake`](https://github.com/LiangliangNan/Easy3D/tree/main/cmake) - CMake-related configuration files
* [`docs`](https://github.com/LiangliangNan/Easy3D/tree/main/docs) - documentation configuration file (Doxygen)
* [`easy3d`](https://github.com/LiangliangNan/Easy3D/tree/main/easy3d) - source code of Easy3D, implementing the Easy3D modules:
  * [`util`](https://github.com/LiangliangNan/Easy3D/tree/main/easy3d/util) - utilities, e.g., logging, file system, progress, timer.
  * [`core`](https://github.com/LiangliangNan/Easy3D/tree/main/easy3d/core) - basic types and data structures, e.g., point cloud, surface mesh, graph, and polyhedron mesh.
  * [`fileio`](https://github.com/LiangliangNan/Easy3D/tree/main/easy3d/fileio) - functionalities for reading/writing data from/into files.
  * [`kdtree`](https://github.com/LiangliangNan/Easy3D/tree/main/easy3d/kdtree) - a collection of kd-trees.
  * [`algo`](https://github.com/LiangliangNan/Easy3D/tree/main/easy3d/algo) - algorithms for geometry processing.
  * [`algo_ext`](https://github.com/LiangliangNan/Easy3D/tree/main/easy3d/algo_ext) - several extended surface mesh processing algorithms (based on CGAL).
  * [`video`](https://github.com/LiangliangNan/Easy3D/tree/main/easy3d/video) - a class that can encode an image sequence into a video.
  * [`renderer`](https://github.com/LiangliangNan/Easy3D/tree/main/easy3d/renderer) - functionalities and algorithms for rendering and visualization, with Vulkan and OpenGL backends.
  * [`gui`](https://github.com/LiangliangNan/Easy3D/tree/main/easy3d/gui) - tools for user interactions, e.g., picking points, faces, or models.
  * [`viewer`](https://github.com/LiangliangNan/Easy3D/tree/main/easy3d/viewer) - a simple viewer and a composite viewer.
  * [`python`](https://github.com/LiangliangNan/Easy3D/tree/main/python) - Python bindings for Easy3D, and examples.
* [`resources`](https://github.com/LiangliangNan/Easy3D/tree/main/resources) - test data, images, shaders, textures, etc.
* [`tests`](https://github.com/LiangliangNan/Easy3D/tree/main/tests) - a collection of test cases
* [`tutorials`](https://github.com/LiangliangNan/Easy3D/tree/main/tutorials) - a collection of examples (with detailed explanations in code)

### Build Easy3D
Like most software, Easy3D depends on some third-party libraries. Easy3D has made this easier for users by including
the source code of most third-party libraries (for the core functionalities and the basic viewer), and it leaves very 
few optional (for a few additional features that are typically not needed by most users). 

#### Rendering backends
Easy3D provides two rendering backends, selected at configure time with the CMake option `Easy3D_ENABLE_VULKAN`:

- **Vulkan** (`Easy3D_ENABLE_VULKAN=ON`, the default): a native Vulkan renderer whose shaders are compiled to SPIR-V 
  at build time. Building it requires a [Vulkan SDK](https://vulkan.lunarg.com/) (on Linux, the `libvulkan-dev` and 
  `glslang-tools` packages suffice); running it requires a Vulkan driver, provided on macOS by MoltenVK (bundled 
  with the SDK).
- **OpenGL** (`Easy3D_ENABLE_VULKAN=OFF`): the original OpenGL backend, which needs no Vulkan SDK.

A build contains exactly one backend, so use separate build directories for the two. The public API — `Viewer`, 
`MultiViewer`, `OffScreen`, drawables, picking, effects, and snapshots — is the same for both backends: client code, 
all tutorials, and the Python bindings build unchanged against either one. The Qt application Mapple uses the OpenGL 
backend (`Easy3D_ENABLE_VULKAN=OFF`).

**Migrating from the OpenGL-only Easy3D:** class names, headers, and the `virtual void draw() const` frame hook are 
unchanged. The one deliberate API change is the viewer constructor: 
`Viewer(title, width, height, samples, full_screen, visible, resizable, options)`, where `options` is `VulkanOptions` 
or `OpenGLOptions` for the configured backend and `samples` defaults to 1. Old positional calls such as 
`Viewer(title, 4, 3, 2, false, false)` still compile but now request a 4x3 window with 2 samples (instead of 4 samples 
with OpenGL 3.2) — update such calls explicitly. See [docs/vulkan.md](docs/vulkan.md) for the full backend and 
migration details.

The **optional** third-party libraries are:
- **[CGAL](https://www.cgal.org/) (optional)**: Easy3D has implemented a few algorithms for advanced surface mesh 
  processing, such as surface reorientation, detecting/resolving duplicate vertices/faces and self-intersection, 
  and clipping/splitting/slicing surface meshes. These features are disabled by default (because most users don't 
  need them). To enable these features, you can switch on the CMake option `Easy3D_ENABLE_CGAL` and make sure 
  CGAL ([v5.1 or later](https://github.com/CGAL/cgal/releases)) is installed and visible to CMake. In case you have
  multiple versions of CGAL on your platform, provide the path of a suitable one to the CMake variable `CGAL_DIR`.

- **[Qt](https://www.qt.io/) (optional)**: Easy3D supports Qt >= v5.8.0 ([v5.14.2](https://download.qt.io/archive/qt/5.14/5.14.2/),
  [v5.12.12](https://download.qt.io/archive/qt/5.12/5.12.12/), and [v6.12.0](https://www.qt.io/download-qt-installer-oss)
  have been tested) for UI creation, helping you develop sophisticated applications for 3D data processing and visualization. 
  The Qt support is disabled by default (because most users don't need it). You can switch on the CMake option `Easy3D_ENABLE_QT` 
  to include the applications that depend on Qt (e.g., 
  [`Mapple`](https://github.com/LiangliangNan/Easy3D/tree/main/applications/Mapple), which uses the OpenGL backend 
  and thus also requires `-DEasy3D_ENABLE_VULKAN=OFF`).
  
To build Easy3D, you need [CMake](https://cmake.org/download/) (`>= 3.12`) and, of course, a compiler that supports `>= C++11`.

Easy3D has been tested on macOS (Xcode >= 8), Windows (MSVC >=2015 `x64`), and Linux (GCC >= 4.8, Clang >= 3.3). Machines 
nowadays typically provide higher [support](https://en.cppreference.com/w/cpp/compiler_support), so you should be able 
to build Easy3D on almost all platforms.

There are many options to build Easy3D. Choose one of the following (not an exhaustive list). All of them build the 
default Vulkan backend; add `-DEasy3D_ENABLE_VULKAN=OFF` to the CMake command to build the OpenGL backend instead. 
Tutorials are not built by default; enable them with `-DEasy3D_BUILD_TUTORIALS=ON`.

- Option 1 (purely on the command line): Use CMake to generate Makefiles and then `make` (on Linux/macOS) or `nmake`(on Windows with Microsoft 
  Visual Studio). 
  - On Linux or macOS, you can simply
    ```bash
        $ cd path-to-root-dir-of-Easy3D
        $ mkdir Release
        $ cd Release
        $ cmake -DCMAKE_BUILD_TYPE=Release ..
        $ make
    ```
  - On Windows with Microsoft Visual Studio, use the `x64 Native Tools Command Prompt for VS XXXX` (**don't** use the x86 one), then
    ```cmd
        $ cd path-to-root-dir-of-Easy3D
        $ mkdir Release
        $ cd Release
        $ cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..
        $ nmake
    ```
  
- Option 2: Use any IDE that can directly handle CMakeLists files to open the `CMakeLists.txt` in the **root** directory of 
  Easy3D. Then you should have obtained a usable project and just build it. I recommend using 
[CLion](https://www.jetbrains.com/clion/) or [QtCreator](https://www.qt.io/product). For Windows users: your IDE must be set for `x64`.
  
- Option 3: Use CMake-Gui to generate project files for your IDE. Then load the project to your IDE and build it. For Windows users: your IDE must be set for `x64`.

Do not have any experience with C/C++ programming? 
Have a look at <a href="https://github.com/LiangliangNan/Easy3D/blob/main/HowToBuild.md">How to build Easy3D step by 
step</a>.

#### Build and use Python bindings
- Please check [here](https://github.com/LiangliangNan/Easy3D/blob/main/python/README.md).

### Test Easy3D
All tests and their CTest registration live in [`tests`](tests/README.md). Configure
with `-DEasy3D_BUILD_TESTS=ON`, build normally, then run
`ctest --test-dir YOUR_BUILD_DIRECTORY --output-on-failure`. This enables all tests
for the configured backend; Vulkan regression tests require validation layers.
The option defaults to `OFF`. Desktop tests run for bounded frames and require no
manual interaction. Python package tests are included when bindings are built.

### Use Easy3D in your project
This is quite easy, like many other open-source libraries :-) 
After you have built Easy3D, you only need to point `Easy3D_DIR` to your `build` (or the installation) directory of Easy3D when doing cmake. Then the requested Easy3D libraries, including directories and relevant compile definitions of Easy3D, are visible and accessible to your project. Below is an example of using the default Easy3D viewer. 
The `CMakeLists.txt` looks like:

```cmake
cmake_minimum_required(VERSION 3.12)
project(MyProject)
set(CMAKE_CXX_STANDARD 11)                       # specify C++ standard
find_package(Easy3D COMPONENTS viewer REQUIRED)  # request Easy3D (recommended to request only needed components)
add_executable(Test main.cpp)                    # create an executable target
target_link_libraries(Test easy3d::viewer)       # link to necessary Easy3D modules (add more if needed, e.g., algo)
```
and the `main.cpp` with minimum code:
```c++
#include <easy3d/viewer/viewer.h>
#include <easy3d/util/initializer.h>

int main(int argc, char** argv) {
    easy3d::initialize();
    easy3d::Viewer viewer("Test");
    return viewer.run();
}
```
Using the Python bindings is also straightforward, for example:
```python
import easy3d

easy3d.initialize()
viewer = easy3d.Viewer("Test")
viewer.add_model(easy3d.resource_directory() + "/data/bunny.ply") # <-- Use your actual file path
viewer.run()
```

### Documentation
 - [Easy3D documentation](https://liangliangnan.github.io/Easy3D/)
 - [Easy3D C++ tutorials](https://liangliangnan.github.io/Easy3D/examples.html)
 - [Easy3D Python bindings documentation](https://liangliangnan.github.io/Easy3D/python)
 - [Easy3D Python bindings tutorials](https://liangliangnan.github.io/Easy3D/python/tutorials/)

The documentation is an ongoing effort with more and more details being added. You can build the latest Easy3D 
documentation from the source code.
Easy3D uses [Doxygen](https://www.doxygen.nl/index.html) (`>= 1.8.3`) to generate documentation from source code. 
To build it from the source code, [install Doxygen](https://www.doxygen.nl/manual/install.html) first. 
Then, switch on the CMake option `Easy3D_BUILD_DOCUMENTATION` and run CMake. Finally, build the `Documentation` 
target to generate it. 

For Python users, you may also be interested in the [tutorials](https://github.com/LiangliangNan/Easy3D/tree/main/python/tutorials).

### Questions, new features, bugs, or contributing to Easy3D
See the [Contribution Guide](https://github.com/LiangliangNan/Easy3D/blob/main/CONTRIBUTING.md) for more information.

### License
Easy3D is free software; you can redistribute it and/or modify it under the terms of the 
GNU General Public License as published by the Free Software Foundation; either version 3
of the License or (at your option) any later version. The full text of the license can be
found in the accompanying 'License' file.

### Acknowledgments
The implementation of Easy3D greatly benefited from and was inspired by existing great open-source libraries, such as
[PMP](http://www.pmp-library.org/), [libQGLViewer](https://github.com/GillesDebunne/libQGLViewer), [Surface mesh](https://opensource.cit-ec.de/projects/surface_mesh),
and [Graphite](http://graphite.gforge.inria.fr/). In particular, the implementation of several surface mesh 
algorithms was taken (with modifications) from PMP, i.e., simplification, subdivision, smoothing, 
parameterization, remeshing, hole filling, geodesic distances, fairing, curvatures, and triangulation. 
We would like to thank the original authors of these projects for their permissive license terms. 
We also thank the users and contributors for reporting/fixing bugs, testing, and providing valuable feedback and suggestions.

### Citation
If you use Easy3D in scientific work, I kindly ask you to cite it:

```bibtex
@article{easy3d2021,
  title = {Easy3{D}: a lightweight, easy-to-use, and efficient {C}++ library for processing and rendering 3{D} data},
  author = {Liangliang Nan},
  journal = {Journal of Open Source Software}，
  year = {2021},
  volume = {6},
  number = {64},
  pages = {3255},
  doi = {10.21105/joss.03255},
  url = {https://doi.org/10.21105/joss.03255}
}
```
---------

Should you have any questions, comments, or suggestions, please contact me at <i>liangliang.nan@gmail.com</i>

[<b><i>Liangliang Nan</i></b>](https://3d.bk.tudelft.nl/liangliang/)

Dec. 8, 2018
