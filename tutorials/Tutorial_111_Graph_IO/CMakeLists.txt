get_filename_component(sample ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(dependencies easy3d::core easy3d::fileio)

set(${sample}_files
        main.cpp
        )

add_sample(${sample} "${${sample}_files}" "${dependencies}")