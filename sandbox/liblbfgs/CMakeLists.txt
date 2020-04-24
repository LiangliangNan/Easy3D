cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(liblbfgs_HEADERS
        include/lbfgs.h
        )

set(liblbfgs_SOURCES
        lib/arithmetic_ansi.h
        lib/arithmetic_sse_double.h
        lib/arithmetic_sse_float.h
        lib/lbfgs.c
        )


add_library(${PROJECT_NAME} STATIC ${liblbfgs_SOURCES} ${liblbfgs_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "3rd_party")

set(LBFGS_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${LBFGS_INCLUDE_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE USE_SSE __SSE__ __SSE2__ __SSE3__ HAVE_EMMINTRIN_H HAVE_XMMINTRIN_H)


