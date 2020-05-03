get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


add_library(${PROJECT_NAME} STATIC
        dict-list.h
        dict.c
        dict.h
        geom.c
        geom.h
        glutess.h
        gluos.h
        memalloc.c
        memalloc.h
        mesh.c
        mesh.h
        normal.c
        normal.h
        priorityq-heap.c
        priorityq-heap.h
        priorityq-sort.h
        priorityq.c
        priorityq.h
        render.c
        render.h
        sweep.c
        sweep.h
        tess.c
        tess.h
        tessmono.c
        tessmono.h
        )

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})