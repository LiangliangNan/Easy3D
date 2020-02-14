cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
        gaussian_noise.h
        point_cloud_normals.h
        point_cloud_poisson_reconstruction.h
        point_cloud_ransac.h
        point_cloud_simplification.h
        surface_mesh_sampler.h
        )

set(${PROJECT_NAME}_SOURCES
        gaussian_noise.cpp
        point_cloud_normals.cpp
        point_cloud_poisson_reconstruction.cpp
        point_cloud_ransac.cpp
        point_cloud_simplification.cpp
        surface_mesh_sampler.cpp
        )

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "easy3d")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_THIRD_PARTY}/RANSAC-1.1)

target_link_libraries(${PROJECT_NAME} core kdtree poisson_recon-9.0.1 RANSAC-1.1)

# It's "Boost", not "BOOST" or "boost". Case matters.
find_package(Boost)
if (Boost_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_BOOST)
endif ()

