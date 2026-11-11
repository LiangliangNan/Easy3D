get_filename_component(example ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(dependencies easy3d::viewer)

set(${example}_files
        main.cpp
        viewer.h
        viewer.cpp
        )

add_example(${example} "${${example}_files}" "${dependencies}")
