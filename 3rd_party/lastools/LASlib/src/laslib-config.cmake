get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${SELF_DIR}/laslib-targets.cmake)
get_filename_component(LASlib_INCLUDE_DIRS "${SELF_DIR}/../../../include/LASlib" ABSOLUTE)
set_property(TARGET LASlib PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${LASlib_INCLUDE_DIRS})

set(LASlib_FOUND true)
