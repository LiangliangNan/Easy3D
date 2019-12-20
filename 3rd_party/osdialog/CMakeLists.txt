cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(osdialog_HEADERS
    osdialog.h
    )

set(osdialog_SOURCES
    osdialog.c
    )


if(APPLE)
    set(osdialog_SOURCES ${osdialog_SOURCES} osdialog_mac.m)
elseif(MSVC)
    set(osdialog_SOURCES ${osdialog_SOURCES} osdialog_win.c)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
    set(osdialog_SOURCES ${osdialog_SOURCES} osdialog_gtk2.c)
    include_directories(${GTK2_INCLUDE_DIRS})
endif()


add_library(${PROJECT_NAME} STATIC ${osdialog_SOURCES} ${osdialog_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "3rd_party")

if (MSVC)
  target_compile_definitions(${PROJECT_NAME} PRIVATE
      _CRT_SECURE_NO_WARNINGS
      _CRT_SECURE_NO_DEPRECATE
      )
endif()
