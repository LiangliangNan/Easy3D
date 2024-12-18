get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_library(3rd_${module} STATIC
    include/GL/eglew.h
    include/GL/glew.h
    include/GL/glxew.h
    include/GL/wglew.h
    src/glew.c
    )
set_target_properties(3rd_${module} PROPERTIES FOLDER "3rd_party")

target_include_directories(3rd_${module} PRIVATE ${ADTREE_glew_INCLUDE_DIR})

target_compile_definitions(3rd_${module} PRIVATE GLEW_STATIC)

