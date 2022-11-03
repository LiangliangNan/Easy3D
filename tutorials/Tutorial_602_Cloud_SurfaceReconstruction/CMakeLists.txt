get_filename_component(sample ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(dependencies easy3d::viewer easy3d::algo)

set(${sample}_files
        main.cpp
        )

add_sample(${sample} "${${sample}_files}" "${dependencies}")
