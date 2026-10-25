# Build documentation without configuring or compiling the C++ project.
# Run from the repository root: cmake -P docs/build.cmake
cmake_minimum_required(VERSION 3.18)

get_filename_component(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
if (NOT DEFINED PROJECT_BINARY_DIR)
    set(PROJECT_BINARY_DIR "${PROJECT_SOURCE_DIR}/build")
endif ()
get_filename_component(PROJECT_BINARY_DIR "${PROJECT_BINARY_DIR}" ABSOLUTE)

# Keep the documentation version in sync with the project's CMake configuration.
file(READ "${PROJECT_SOURCE_DIR}/CMakeLists.txt" project_cmake)
foreach (part MAJOR MINOR PATCH)
    string(REGEX MATCH "set\\(Easy3D_VERSION_${part} +([0-9]+)\\)" match "${project_cmake}")
    if (NOT match)
        message(FATAL_ERROR "Cannot read Easy3D_VERSION_${part} from CMakeLists.txt")
    endif ()
    set(version_${part} "${CMAKE_MATCH_1}")
endforeach ()
set(Easy3D_VERSION "${version_MAJOR}.${version_MINOR}.${version_PATCH}")

find_program(DOXYGEN_EXECUTABLE doxygen REQUIRED)
file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}")
configure_file("${CMAKE_CURRENT_LIST_DIR}/Doxyfile.in" "${PROJECT_BINARY_DIR}/Doxyfile" @ONLY)
execute_process(
    COMMAND "${DOXYGEN_EXECUTABLE}" "${PROJECT_BINARY_DIR}/Doxyfile"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    RESULT_VARIABLE result
)
if (NOT result EQUAL 0)
    message(FATAL_ERROR "Doxygen failed: ${result}")
endif ()
if (NOT EXISTS "${PROJECT_BINARY_DIR}/doc/html/index.html")
    message(FATAL_ERROR "Doxygen did not generate doc/html/index.html")
endif ()
message(STATUS "Documentation: ${PROJECT_BINARY_DIR}/doc/html/index.html")
