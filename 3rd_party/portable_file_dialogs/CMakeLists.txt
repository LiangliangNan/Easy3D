cmake_minimum_required(VERSION 3.1.0)

set (CMAKE_CXX_STANDARD 11)

project(portable_file_dialogs VERSION 1.00 LANGUAGES CXX)

add_executable(${PROJECT_NAME}
                example.cpp
)
