get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

include(BackwardConfig.cmake)

# check if compiler is nvcc or nvcc_wrapper
set(COMPILER_IS_NVCC false)
get_filename_component(COMPILER_NAME ${CMAKE_CXX_COMPILER} NAME)
if (COMPILER_NAME MATCHES "^nvcc")
  set(COMPILER_IS_NVCC true)
endif()

if (DEFINED ENV{OMPI_CXX} OR DEFINED ENV{MPICH_CXX})
   if ( ($ENV{OMPI_CXX} MATCHES "nvcc") OR ($ENV{MPICH_CXX} MATCHES "nvcc") )
     set(COMPILER_IS_NVCC true)
   endif()
endif()

# set CXX standard
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD 11)
if (${COMPILER_IS_NVCC})
  # GNU CXX extensions are not supported by nvcc
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

###############################################################################
# COMPILER FLAGS
###############################################################################

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	if (NOT ${COMPILER_IS_NVCC})
	  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic-errors")
	endif()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

add_library(3rd_${module} STATIC backward.cpp backward.hpp)
target_compile_definitions(3rd_${module} PRIVATE ${BACKWARD_DEFINITIONS})
target_include_directories(3rd_${module} PRIVATE ${BACKWARD_INCLUDE_DIRS})

find_package(Threads REQUIRED)
target_link_libraries(3rd_${module} PUBLIC Threads::Threads)
set_target_properties(3rd_${module} PROPERTIES FOLDER "3rd_party")
set_target_properties(3rd_${module} PROPERTIES POSITION_INDEPENDENT_CODE ON)
if (MSVC)
	target_compile_definitions(3rd_${module} PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
endif()


if (NOT ${Easy3D_BUILD_SHARED_LIBS})
    install(TARGETS 3rd_${module}
            # IMPORTANT: Add the library to the "export-set"
            EXPORT Easy3DTargets
            LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
            COMPONENT dev)
endif ()
