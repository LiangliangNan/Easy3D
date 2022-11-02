cmake_minimum_required(VERSION 3.12)

project(3rd_glew)

add_library(3rd_glew STATIC src/glew.c)

set_target_properties(3rd_glew PROPERTIES FOLDER "3rd_party")

set(GLEW_INCLUDE_DIR  ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(3rd_glew PRIVATE ${GLEW_INCLUDE_DIR})

target_compile_definitions(3rd_glew PUBLIC GLEW_STATIC GLEW_NO_GLU)

set (OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL libraries: ${OPENGL_LIBRARIES}")
target_link_libraries(3rd_glew PUBLIC ${OPENGL_LIBRARIES})


set_target_properties(3rd_glew PROPERTIES PUBLIC_HEADER "include/GL/glew.h")
install(TARGETS 3rd_glew
        # IMPORTANT: Add the library to the "export-set"
        EXPORT Easy3DTargets
        #RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT lib
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/3rd_party/glew/include/GL/${module}"
        COMPONENT dev)