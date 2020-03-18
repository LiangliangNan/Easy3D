cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(libtess)


################################################################################

set(CMAKE_CXX_STANDARD 11)

################################################################################


add_library(${PROJECT_NAME} STATIC
        source/bucketalloc.h
        source/dict.h
        source/geom.h
        source/mesh.h
        source/priorityq.h
        source/sweep.h
        source/tess.h
        source/bucketalloc.c
        source/dict.c
        source/geom.c
        source/mesh.c
        source/priorityq.c
        source/sweep.c
        source/tess.c
        )
