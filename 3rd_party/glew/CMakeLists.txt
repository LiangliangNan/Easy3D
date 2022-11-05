get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(${module}_HEADERS
		include/GL/glew.h
		)

set(${module}_SOURCES
		src/glew.c
		)

add_3rdparty_module(3rd_${module} "${${module}_SOURCES}" "${${module}_HEADERS}")
target_include_directories(3rd_${module} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_definitions(3rd_${module} PUBLIC GLEW_STATIC GLEW_NO_GLU)

set (OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL libraries: ${OPENGL_LIBRARIES}")
target_link_libraries(3rd_${module} PUBLIC ${OPENGL_LIBRARIES})


set_target_properties(3rd_${module} PROPERTIES PUBLIC_HEADER "include/GL/glew.h")
install(TARGETS 3rd_${module}
        # IMPORTANT: Add the library to the "export-set"
        EXPORT Easy3DTargets
		LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
        PUBLIC_HEADER DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/3rd_party/glew/include/GL"
		COMPONENT dev)