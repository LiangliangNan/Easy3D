cmake_minimum_required(VERSION 3.1)
project(Easy3D)

################################################################################

set (CMAKE_CXX_STANDARD 11)

################################################################################

# Detects whether this is a top-level project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(EASY3D_TOPLEVEL_PROJECT ON)
else()
	set(EASY3D_TOPLEVEL_PROJECT OFF)
endif()

# Build examples and third-parties
option(EASY3D_BUILD_EXAMPLES  "Build Easy3D examples" 		ON)

################################################################################

### Configuration
set(EASY3D_ROOT "${CMAKE_CURRENT_LIST_DIR}")
set(EASY3D_INCLUDE_DIR "${EASY3D_ROOT}")
set(EASY3D_SOURCE_DIR "${EASY3D_ROOT}/core" "${EASY3D_ROOT}/model" "${EASY3D_ROOT}/imgui")
set(EASY3D_EXTERNAL "${EASY3D_ROOT}/3rd_party")

set(EASY3D_glew_INCLUDE_DIR "${EASY3D_EXTERNAL}/glew/include")
set(EASY3D_glfw_INCLUDE_DIR "${EASY3D_EXTERNAL}/glfw/include")
set(EASY3D_imgui_INCLUDE_DIR "${EASY3D_EXTERNAL}/imgui" "${EASY3D_EXTERNAL}/imgui/impl")


### conditionally compile certain modules depending on libraries found on the system
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)


################################################################################


set(GLEW_STATIC 1)

##for C
#set(CMAKE_C_FLAGS_DEBUG _DEBUG)
#set(CMAKE_C_FLAGS_RELWITHDEBINFO _DEBUG)
##For C++
#set(CMAKE_CXX_FLAGS_DEBUG _DEBUG)
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO _DEBUG)

set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG} -_DEBUG)

################################################################################


add_subdirectory(3rd_party)
add_subdirectory(easy3d)


if(EASY3D_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
