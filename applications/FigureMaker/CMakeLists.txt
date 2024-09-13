get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_executable(${name} main.cpp viewer.h viewer.cpp)
set_target_properties(${name} PROPERTIES FOLDER "applications")
target_include_directories(${name} PRIVATE ${Easy3D_INCLUDE_DIR})
target_link_libraries(${name} PRIVATE easy3d::viewer)

target_compile_definitions(${name} PRIVATE "APP_TITLE=\"${name}\"")



