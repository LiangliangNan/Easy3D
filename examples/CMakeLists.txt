cmake_minimum_required(VERSION 3.1)

add_subdirectory("Example_01_PointCloud")
add_subdirectory("Example_02_Mesh")
add_subdirectory("Example_03_FileIO")
add_subdirectory("Example_03_Property")
add_subdirectory("Example_04_Viewer")
add_subdirectory("Example_05_Drawable")
add_subdirectory("Example_06_ScalarField")
add_subdirectory("Example_07_VectorField")


#get_directory_property(subdir_list DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} SUBDIRECTORIES)
#foreach(subdir ${subdir_list})
#    add_subdirectory("${subdir}")
#    message(STATUS "${subdir}")
#endforeach()
