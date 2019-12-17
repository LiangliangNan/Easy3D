cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(tinyobjloader_HEADERS
    tiny_obj_loader.h
    )

set(tinyobjloader_SOURCES
    tiny_obj_loader.cc
    )


add_library(${PROJECT_NAME} STATIC ${tinyobjloader_SOURCES} ${tinyobjloader_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "3rd_party")

if (MSVC)
  target_compile_definitions(${PROJECT_NAME} PRIVATE
      _CRT_SECURE_NO_WARNINGS
      _CRT_SECURE_NO_DEPRECATE
      )
endif()
