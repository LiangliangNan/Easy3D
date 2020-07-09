get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


add_library(${PROJECT_NAME} STATIC
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

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "3rd_party")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
