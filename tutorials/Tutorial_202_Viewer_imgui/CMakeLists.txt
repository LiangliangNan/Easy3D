get_filename_component(example ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(dependencies 3rd_imgui easy3d::core easy3d::renderer easy3d::viewer easy3d::util)

set(${example}_files
        main.cpp
        viewer.h
        viewer.cpp
        )

add_example(${example} "${${example}_files}" "${dependencies}")