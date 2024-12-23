cmake_minimum_required(VERSION 3.12)
project(PyEasy3D)

set(CMAKE_CXX_STANDARD 14)                       # specify C++ standard

find_package(Easy3D COMPONENTS viewer REQUIRED)  # request Easy3D (recommended to request only needed components)
find_package(pybind11 REQUIRED)

pybind11_add_module(easy3d MODULE
        easy3d.cpp
        bindings/util.cpp
        bindings/core.cpp
        bindings/fileio.cpp
        bindings/algo.cpp
        bindings/viewer.cpp
        )
target_link_libraries(easy3d PRIVATE easy3d::util easy3d::core easy3d::renderer easy3d::fileio easy3d::algo easy3d::viewer)
target_compile_definitions(easy3d PRIVATE PYBIND11_SIMPLE_GIL_SAFE_ABI) # to use the Python Stable ABI
# By default, the output .so will include the Python version. Use a CMake command to rename it to a version-independent name
set_target_properties(easy3d PROPERTIES SUFFIX ".so")
set_target_properties(easy3d PROPERTIES FOLDER "python")