get_filename_component(example ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(dependencies easy3d::viewer)
if(TARGET easy3d::gui)   # the picker module is not built with Vulkan yet
    list(APPEND dependencies easy3d::gui)
endif()

set(${example}_files
        main.cpp
        viewer.h
        viewer.cpp
        )

add_example(${example} "${${example}_files}" "${dependencies}")

