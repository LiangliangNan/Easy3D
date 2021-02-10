cmake_minimum_required(VERSION 3.1)

project(3rd_glutess)

add_library(3rd_glutess STATIC
        glutess.h
        source/dict-list.h
        source/dict.c
        source/dict.h
        source/geom.c
        source/geom.h
        source/memalloc.c
        source/memalloc.h
        source/mesh.c
        source/mesh.h
        source/normal.c
        source/normal.h
        source/priorityq-heap.c
        source/priorityq-heap.h
        source/priorityq-sort.h
        source/priorityq.c
        source/priorityq.h
        source/render.c
        source/render.h
        source/sweep.c
        source/sweep.h
        source/tess.c
        source/tess.h
        source/tessmono.c
        source/tessmono.h
        )

set_target_properties(3rd_glutess PROPERTIES FOLDER "3rd_party")

target_include_directories(3rd_glutess PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})