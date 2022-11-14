get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(${module}_HEADERS
		include/GL/glew.h
		)

set(${module}_SOURCES
		src/glew.c
		)


add_library(3rd_${module} SHARED "${${module}_SOURCES}" "${${module}_HEADERS}")
    set_target_properties(3rd_${module} PROPERTIES FOLDER "3rd_party")
    set_target_properties(3rd_${module} PROPERTIES POSITION_INDEPENDENT_CODE ON)
if (MSVC)
    target_compile_definitions(3rd_${module} PRIVATE WIN32_LEAN_AND_MEAN VC_EXTRALEAN _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
endif()

target_include_directories(3rd_${module} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_definitions(3rd_${module} PRIVATE GLEW_BUILD)
target_compile_definitions(3rd_${module} PUBLIC GLEW_NO_GLU)

if(LINUX)
    set (OpenGL_GL_PREFERENCE GLVND)
endif()
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL library: ${OPENGL_gl_LIBRARY}")
target_link_libraries(3rd_${module} PUBLIC ${OPENGL_gl_LIBRARY})


set_target_properties(3rd_${module} PROPERTIES PUBLIC_HEADER "include/GL/glew.h")
install(TARGETS 3rd_${module}
        # IMPORTANT: Add the library to the "export-set"
        EXPORT Easy3DTargets
		LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
        PUBLIC_HEADER DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/3rd_party/glew/include/GL"
		COMPONENT dev)