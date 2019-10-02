cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
    point_cloud_normals.h
    point_cloud_poisson_reconstruction.h
#   point_cloud_qualities.h
    point_cloud_ransac.h
    surface_mesh_sampler.h
    )

set(${PROJECT_NAME}_SOURCES
    point_cloud_normals.cpp
    point_cloud_poisson_reconstruction.cpp
#   point_cloud_qualities.cpp
    point_cloud_ransac.cpp
    surface_mesh_sampler.cpp
    )

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

	
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS} )

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "easy3d")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_THIRD_PARTY}/RANSAC-1.1)

target_link_libraries(${PROJECT_NAME} core kdtree poisson_recon-9.0.1 RANSAC-1.1)


find_package(Boost)
if (Boost_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_BOOST)
endif()

