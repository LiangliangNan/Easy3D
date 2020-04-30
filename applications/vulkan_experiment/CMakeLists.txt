get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

################################################################################

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

################################################################################

# Config vulkan
find_package(Vulkan REQUIRED FATAL_ERROR)
message(STATUS "Vulkan Include dirs: ${Vulkan_INCLUDE_DIRS} and libs: ${Vulkan_LIBRARIES}")

add_executable(${PROJECT_NAME}
#        application.h
#        application.cpp
        main.cpp
        )

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models/viking_room.obj DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}/models)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models/viking_room.png DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}/textures)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vert.spv DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}/shaders)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/frag.spv DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}/shaders)

