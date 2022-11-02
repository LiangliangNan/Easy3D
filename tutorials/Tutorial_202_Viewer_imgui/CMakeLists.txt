get_filename_component(sample ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(dependencies 3rd_imgui easy3d::core easy3d::renderer easy3d::viewer easy3d::util)

set(${sample}_files
        main.cpp
        viewer.h
        viewer.cpp
        )

add_sample(${sample} "${${sample}_files}" "${dependencies}")
