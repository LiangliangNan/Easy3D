cmake_minimum_required(VERSION 3.1)

project(3rd_triangle)

add_library(3rd_triangle STATIC triangle.h triangle.c)

set_target_properties(3rd_triangle PROPERTIES FOLDER "3rd_party")

#Set properties on a target.
#We use this here to set -DTETLIBRARY for when compiling the
#library
set_target_properties(3rd_triangle PROPERTIES "COMPILE_DEFINITIONS" TRILIBRARY)


if (MSVC)
    target_compile_definitions(3rd_triangle PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
        )
endif()