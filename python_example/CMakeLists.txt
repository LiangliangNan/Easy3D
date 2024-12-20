cmake_minimum_required(VERSION 3.12)
project(Easy3DLib)

set(CMAKE_CXX_STANDARD 11)                       # specify C++ standard

find_package(Easy3D COMPONENTS viewer REQUIRED)  # request Easy3D (recommended to request only needed components)

find_package(pybind11 REQUIRED)

#set(CMAKE_CXX_STANDARD 20)
#set(PYBIND11_PYTHON_VERSION 3.6)
#set(CMAKE_CXX_FLAGS "-Wall -Wextra -fPIC")

pybind11_add_module(${PROJECT_NAME} easy3d.h easy3d.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${PYBIND11_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE easy3d::viewer)       # link to necessary Easy3D modules (add more if needed, e.g., algo)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "python_example")


# add the executable
# add_executable(${PROJECT_NAME} main.cpp)
# target_include_directories( ${PROJECT_NAME} PRIVATE)
# target_link_libraries(${PROJECT_NAME} PRIVATE Eigen3::Eigen)