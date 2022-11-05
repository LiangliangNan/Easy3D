get_filename_component(example ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(dependencies easy3d::core easy3d::fileio)

set(${example}_files
        main.cpp
        )

add_example(${example} "${${example}_files}" "${dependencies}")