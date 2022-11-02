cmake_minimum_required(VERSION 3.12)

project(3rd_rply)

add_library(3rd_rply STATIC rply.c rply.h)

set_target_properties(3rd_rply PROPERTIES FOLDER "3rd_party")


install(TARGETS 3rd_rply
        # IMPORTANT: Add the library to the "export-set"
        EXPORT Easy3DTargets
        #RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION ${INSTALL_LIB_DIR} COMPONENT lib
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/${module}"
        COMPONENT dev)