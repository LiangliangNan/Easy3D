cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -fPIC")

add_subdirectory(glew)
add_subdirectory(glfw)
add_subdirectory(imgui)

add_subdirectory(rply)
add_subdirectory(lastools)

add_subdirectory(kdtree)

add_subdirectory(ransac)
add_subdirectory(poisson)

add_subdirectory(triangle)
add_subdirectory(tetgen)
add_subdirectory(glutess)

add_subdirectory(backward)
add_subdirectory(easyloggingpp)



