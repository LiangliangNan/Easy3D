cmake_minimum_required(VERSION 3.12)
project(Easy3DBindings)

set(CMAKE_CXX_STANDARD 14)                       # specify C++ standard

find_package(Easy3D COMPONENTS viewer REQUIRED)  # request Easy3D (recommended to request only needed components)
find_package(pybind11 REQUIRED)

pybind11_add_module(easy3d_util MODULE util_bindings.cpp)
target_link_libraries(easy3d_util PRIVATE easy3d::util)
set_target_properties(easy3d_util PROPERTIES FOLDER "python")

pybind11_add_module(easy3d_core MODULE core_bindings.cpp)
target_link_libraries(easy3d_core PRIVATE easy3d::core easy3d::renderer)
set_target_properties(easy3d_core PROPERTIES FOLDER "python")

pybind11_add_module(easy3d_fileio MODULE fileio_bindings.cpp)
target_link_libraries(easy3d_fileio PRIVATE easy3d::fileio)
set_target_properties(easy3d_fileio PROPERTIES FOLDER "python")

pybind11_add_module(easy3d_algo MODULE algo_bindings.cpp)
target_link_libraries(easy3d_algo PRIVATE easy3d::algo)
set_target_properties(easy3d_algo PROPERTIES FOLDER "python")

pybind11_add_module(easy3d_viewer MODULE viewer_bindings.cpp)
target_link_libraries(easy3d_viewer PRIVATE easy3d::viewer)
set_target_properties(easy3d_viewer PROPERTIES FOLDER "python")


# add the executable
# add_executable(${PROJECT_NAME} main.cpp)
# target_include_directories( ${PROJECT_NAME} PRIVATE)
# target_link_libraries(${PROJECT_NAME} PRIVATE Eigen3::Eigen)