cmake_minimum_required(VERSION 3.12)
project(PyEasy3D)

set(CMAKE_CXX_STANDARD 14)                       # specify C++ standard

find_package(Easy3D COMPONENTS viewer REQUIRED)  # request Easy3D (recommended to request only needed components)
find_package(pybind11 REQUIRED)

pybind11_add_module(easy3d_util MODULE bindings/util.cpp)
target_link_libraries(easy3d_util PRIVATE easy3d::util)
target_compile_definitions(easy3d_util PRIVATE PYBIND11_SIMPLE_GIL_SAFE_ABI) # to use the Python Stable ABI
# By default, the output .so will include the Python version. Use a CMake command to rename it to a version-independent name
set_target_properties(easy3d_util PROPERTIES SUFFIX ".so")
set_target_properties(easy3d_util PROPERTIES FOLDER "python")

pybind11_add_module(easy3d_core MODULE bindings/core.cpp)
target_link_libraries(easy3d_core PRIVATE easy3d::core easy3d::renderer)
target_compile_definitions(easy3d_core PRIVATE PYBIND11_SIMPLE_GIL_SAFE_ABI) # to use the Python Stable ABI
# By default, the output .so will include the Python version. Use a CMake command to rename it to a version-independent name
set_target_properties(easy3d_core PROPERTIES SUFFIX ".so")
set_target_properties(easy3d_core PROPERTIES FOLDER "python")

pybind11_add_module(easy3d_fileio MODULE bindings/fileio.cpp)
target_link_libraries(easy3d_fileio PRIVATE easy3d::fileio)
target_compile_definitions(easy3d_fileio PRIVATE PYBIND11_SIMPLE_GIL_SAFE_ABI) # to use the Python Stable ABI
# By default, the output .so will include the Python version. Use a CMake command to rename it to a version-independent name
set_target_properties(easy3d_fileio PROPERTIES SUFFIX ".so")
set_target_properties(easy3d_fileio PROPERTIES FOLDER "python")

pybind11_add_module(easy3d_algo MODULE bindings/algo.cpp)
target_link_libraries(easy3d_algo PRIVATE easy3d::algo)
target_compile_definitions(easy3d_algo PRIVATE PYBIND11_SIMPLE_GIL_SAFE_ABI) # to use the Python Stable ABI
# By default, the output .so will include the Python version. Use a CMake command to rename it to a version-independent name
set_target_properties(easy3d_algo PROPERTIES SUFFIX ".so")
set_target_properties(easy3d_algo PROPERTIES FOLDER "python")

pybind11_add_module(easy3d_viewer MODULE bindings/viewer.cpp)
target_link_libraries(easy3d_viewer PRIVATE easy3d::viewer)
target_compile_definitions(easy3d_viewer PRIVATE PYBIND11_SIMPLE_GIL_SAFE_ABI) # to use the Python Stable ABI
# By default, the output .so will include the Python version. Use a CMake command to rename it to a version-independent name
set_target_properties(easy3d_viewer PROPERTIES SUFFIX ".so")
set_target_properties(easy3d_viewer PROPERTIES FOLDER "python")


# add the executable
# add_executable(${PROJECT_NAME} main.cpp)
# target_include_directories( ${PROJECT_NAME} PRIVATE)
# target_link_libraries(${PROJECT_NAME} PRIVATE Eigen3::Eigen)