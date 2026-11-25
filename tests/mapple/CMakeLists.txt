# Generated-path provenance is application bookkeeping and needs no backend context.
add_executable(easy3d_generated_path_test
        generated_path_test.cpp
        ../../applications/Mapple/camera_path.cpp)
target_include_directories(easy3d_generated_path_test PRIVATE ../../applications/Mapple)
target_link_libraries(easy3d_generated_path_test PRIVATE easy3d::renderer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.generatedpath COMMAND easy3d_generated_path_test)
    set_tests_properties(vulkan.generatedpath PROPERTIES TIMEOUT 120)
else()
    add_test(NAME opengl.generatedpath COMMAND easy3d_generated_path_test)
    set_tests_properties(opengl.generatedpath PROPERTIES TIMEOUT 120)
endif()

include(../properties.cmake)
