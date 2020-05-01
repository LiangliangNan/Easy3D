get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

################################################################################

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

################################################################################

# Config vulkan
find_package(Vulkan QUIET)
if (Vulkan_FOUND)
    message(STATUS "Vulkan Include dirs: ${Vulkan_INCLUDE_DIRS} and libs: ${Vulkan_LIBRARIES}")
    add_executable(${PROJECT_NAME}
            application.h
            application.cpp
            main.cpp
            )
    target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR} ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES} glfw core viewer util)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models/viking_room.obj DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/data/viking_room)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models/viking_room.png DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/data/viking_room)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/vert.spv DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/shaders/vulkan)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/frag.spv DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources/shaders/vulkan)
else()
    message(WARNING "Vulkan not found, so the Vulkan examples are disabled")
endif()

