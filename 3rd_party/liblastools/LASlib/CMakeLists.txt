cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set(CMAKE_SUPPRESS_REGENERATION true)
project("LASlib")

if (!MSVC)
	add_compile_options(-O3 -Wall -Wno-strict-aliasing)
else()
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(LASlib/src)
add_subdirectory(LASlib/example)
