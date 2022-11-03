cmake_minimum_required(VERSION 3.12)

project(3rd_rply)

add_library(3rd_rply STATIC rply.c rply.h)
set_target_properties(3rd_rply PROPERTIES FOLDER "3rd_party")
set_target_properties(3rd_rply PROPERTIES POSITION_INDEPENDENT_CODE ON)
if (MSVC)
	target_compile_definitions(3rd_rply PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
endif()

install(TARGETS 3rd_rply
        # IMPORTANT: Add the library to the "export-set"
        EXPORT Easy3DTargets
        #RUNTIME DESTINATION "${Easy3D_INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
        PUBLIC_HEADER DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/${module}"
        COMPONENT dev)