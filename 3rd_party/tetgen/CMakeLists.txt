cmake_minimum_required(VERSION 3.12)

project(3rd_tetgen)

add_library(3rd_tetgen STATIC tetgen.cxx predicates.cxx)

#Set properties on a target. 
#We use this here to set -DTETLIBRARY for when compiling the
#library
set_target_properties(3rd_tetgen PROPERTIES COMPILE_DEFINITIONS TETLIBRARY)

set_target_properties(3rd_tetgen PROPERTIES FOLDER "3rd_party")


install(TARGETS 3rd_tetgen
        # IMPORTANT: Add the library to the "export-set"
        EXPORT Easy3DTargets
        #RUNTIME DESTINATION "${Easy3D_INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
        PUBLIC_HEADER DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/${module}"
        COMPONENT dev)