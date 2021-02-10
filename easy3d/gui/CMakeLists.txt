cmake_minimum_required(VERSION 3.1)

get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJECT_NAME "easy3d_${MODULE_NAME}")
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
        picker.h
        picker_model.h
        picker_point_cloud.h
        picker_surface_mesh.h
        )

set(${PROJECT_NAME}_SOURCES
        picker.cpp
        picker_model.cpp
        picker_point_cloud.cpp
        picker_surface_mesh.cpp
        )

add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_SOURCES}
        ${${PROJECT_NAME}_HEADERS}
        )

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "easy3d")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC easy3d_core easy3d_util easy3d_renderer)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif ()


# The header files to be installed
file(GLOB MODULE_HEADER_FILES ${CMAKE_CURRENT_LIST_DIR}/*.h)
# Allow to install the lib and the public headers
install(TARGETS ${PROJECT_NAME}
        # IMPORTANT: Add the library to the "export-set"
        EXPORT Easy3DTargets
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/easy3d/${MODULE_NAME}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}/${PROJECT_NAME}"
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin)
# Install the header files
install(
        FILES ${MODULE_HEADER_FILES}
        DESTINATION "${INSTALL_INCLUDE_DIR}/easy3d/${MODULE_NAME}"
        COMPONENT dev
)
