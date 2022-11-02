cmake_minimum_required(VERSION 3.12)

project(3rd_triangle)

add_library(3rd_triangle STATIC triangle.h triangle.c)

set_target_properties(3rd_triangle PROPERTIES FOLDER "3rd_party")

#Set properties on a target.
#We use this here to set -DTETLIBRARY for when compiling the
#library
set_target_properties(3rd_triangle PROPERTIES "COMPILE_DEFINITIONS" TRILIBRARY)


install(TARGETS 3rd_triangle
        # IMPORTANT: Add the library to the "export-set"
        EXPORT Easy3DTargets
        #RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT lib
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/${module}"
        COMPONENT dev)