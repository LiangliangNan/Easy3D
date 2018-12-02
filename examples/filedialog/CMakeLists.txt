cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


file(GLOB SOURCES ./*.cpp ./*.c)
file(GLOB HEADERS ./*.h)
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
