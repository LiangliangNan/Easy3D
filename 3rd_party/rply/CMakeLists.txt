cmake_minimum_required(VERSION 3.1)

project(3rd_rply)

add_library(3rd_rply STATIC rply.c rply.h)

set_target_properties(3rd_rply PROPERTIES FOLDER "3rd_party")

if (MSVC)
    target_compile_definitions(3rd_rply PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
        )
endif()