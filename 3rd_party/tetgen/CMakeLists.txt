cmake_minimum_required(VERSION 3.1)

project(3rd_tetgen)

add_library(3rd_tetgen STATIC tetgen.cxx predicates.cxx)

#Set properties on a target. 
#We use this here to set -DTETLIBRARY for when compiling the
#library
set_target_properties(3rd_tetgen PROPERTIES COMPILE_DEFINITIONS TETLIBRARY)

set_target_properties(3rd_tetgen PROPERTIES FOLDER "3rd_party")