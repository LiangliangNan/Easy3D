find_package(Vulkan REQUIRED)
find_program(Easy3D_GLSLANG_VALIDATOR NAMES glslangValidator
        HINTS "$ENV{VULKAN_SDK}/bin" "$ENV{VULKAN_SDK}/Bin")
if(NOT Easy3D_GLSLANG_VALIDATOR)
    message(FATAL_ERROR "The Vulkan backend requires glslangValidator at build time (Vulkan SDK or glslang-tools). Installed applications do not require it.")
endif()

set(shader_headers)
foreach(shader scene.vert scene.frag points.vert points.frag lines.vert lines.frag wide_lines.vert wide_lines.frag overlay.vert overlay.frag point_selection.comp)
    string(REPLACE "." "_" symbol "${shader}")
    set(source "${Easy3D_ROOT}/resources/shaders/vulkan/${shader}")
    set(binary "${CMAKE_CURRENT_BINARY_DIR}/${symbol}.spv")
    set(header "${CMAKE_CURRENT_BINARY_DIR}/${symbol}.inc")
    add_custom_command(OUTPUT "${header}" "${binary}"
            COMMAND "${Easy3D_GLSLANG_VALIDATOR}" -V --target-env vulkan1.2 -o "${binary}" "${source}"
            COMMAND "${CMAKE_COMMAND}" "-DINPUT=${binary}" "-DOUTPUT=${header}"
                    "-DSYMBOL=${symbol}" -P "${Easy3D_ROOT}/cmake/EmbedSpirv.cmake"
            DEPENDS "${source}" "${Easy3D_ROOT}/cmake/EmbedSpirv.cmake"
            VERBATIM)
    list(APPEND shader_headers "${header}")
endforeach()

add_module(vulkan "render_device.h;text_renderer.h" "render_device.cpp;text_renderer.cpp;clipping.cpp;vulkan/device.cpp;${shader_headers}"
        "Vulkan::Vulkan" "easy3d::core")
target_include_directories(easy3d_vulkan PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
install(TARGETS easy3d_vulkan EXPORT Easy3DTargets
        RUNTIME DESTINATION "${Easy3D_INSTALL_BIN_DIR}"
        LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}"
        ARCHIVE DESTINATION "${Easy3D_INSTALL_LIB_DIR}")
install(FILES render_device.h text_renderer.h DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/easy3d/graphics")
install(FILES vulkan/surface.h DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/easy3d/graphics/vulkan")

if(Easy3D_BUILD_VULKAN_TESTS)
    add_executable(easy3d_vulkan_test tests/render_test.cpp)
    target_link_libraries(easy3d_vulkan_test PRIVATE easy3d::vulkan)
    set(validation_args)
    if(Easy3D_VULKAN_TEST_VALIDATION)
        set(validation_args --validation)
    endif()
    add_executable(easy3d_vulkan_text_test tests/text_test.cpp)
    target_link_libraries(easy3d_vulkan_text_test PRIVATE easy3d::vulkan easy3d::util)
    add_test(NAME vulkan.text COMMAND easy3d_vulkan_text_test ${validation_args})
    set_tests_properties(vulkan.text PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_depth_overlay_test tests/depth_overlay_test.cpp)
    target_link_libraries(easy3d_vulkan_depth_overlay_test PRIVATE easy3d::vulkan)
    add_test(NAME vulkan.depthoverlay COMMAND easy3d_vulkan_depth_overlay_test ${validation_args})
    set_tests_properties(vulkan.depthoverlay PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_point_selection_test tests/point_selection_test.cpp)
    target_link_libraries(easy3d_vulkan_point_selection_test PRIVATE easy3d::vulkan)
    add_test(NAME vulkan.pointselection COMMAND easy3d_vulkan_point_selection_test ${validation_args})
    set_tests_properties(vulkan.pointselection PROPERTIES TIMEOUT 120)
    add_test(NAME vulkan.render COMMAND easy3d_vulkan_test ${validation_args})
    set_tests_properties(vulkan.render PROPERTIES TIMEOUT 120)
endif()
