cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# find Qt5Core
find_package(Qt5 COMPONENTS Core Gui)

if (NOT Qt5Core_FOUND)
    message(WARNING "${PROJECT_NAME} requires Qt but Qt was not found. You can set 'Qt5_DIR' to the "
            "directory containing 'Qt5Config.cmake' or 'qt5-config.cmake'. "
            "Optionally, you can set the Qt5 root directory 'QT5_ROOT_PATH' to the directory "
            "containing the 'bin' folder.")
    return()
endif ()


if(WIN32 OR MSVC)
    add_executable(${PROJECT_NAME} main.cpp windeployqt/main.cpp)
elseif(APPLE)
    add_executable(${PROJECT_NAME} main.cpp macdeployqt/main.cpp macdeployqt/shared.h macdeployqt/shared.cpp)
    target_link_libraries(${PROJECT_NAME} "-framework CoreFoundation")
else()
    set(CMAKE_AUTOMOC ON)

    # expose version data as compiler definition
    add_definitions("-DLINUXDEPLOYQT_VERSION=\"2021-01-27\"")
    add_definitions("-DLINUXDEPLOYQT_GIT_COMMIT=\"2021-01-27\"")
    add_definitions("-DBUILD_DATE=\"2021-01-27\"")
    add_definitions("-DBUILD_NUMBER=\"2021-01-27\"")

    # update excludelist
    message(STATUS "Updating excludelist...")
    execute_process(
            COMMAND bash ${CMAKE_CURRENT_LIST_DIR}/linuxdeployqt/generate-excludelist.sh
            OUTPUT_VARIABLE EXCLUDELIST
            TIMEOUT 10
            RESULT_VARIABLE EXCLUDELIST_RESULT
    )
    if(NOT EXCLUDELIST_RESULT EQUAL 0)
        message(WARNING "Updating excludelist failed, using outdated copy")
    endif()
    mark_as_advanced(EXCLUDELIST EXCLUDELIST_RESULT)

    add_executable(${PROJECT_NAME} main.cpp linuxdeployqt/main.cpp linuxdeployqt/shared.h linuxdeployqt/shared.cpp linuxdeployqt/excludelist.h)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DEXCLUDELIST="${EXCLUDELIST}")
endif()

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui)

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_DEPRECATED_WARNINGS)