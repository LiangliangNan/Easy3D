find_package(Vulkan REQUIRED)
find_program(Easy3D_GLSLANG_VALIDATOR NAMES glslangValidator
        HINTS "$ENV{VULKAN_SDK}/bin" "$ENV{VULKAN_SDK}/Bin")
if(NOT Easy3D_GLSLANG_VALIDATOR)
    message(FATAL_ERROR "The Vulkan backend requires glslangValidator at build time (Vulkan SDK or glslang-tools). Installed applications do not require it.")
endif()

set(shader_headers)
foreach(stage vert frag)
    set(source "${Easy3D_ROOT}/resources/shaders/vulkan/scene.${stage}")
    set(binary "${CMAKE_CURRENT_BINARY_DIR}/scene_${stage}.spv")
    set(header "${CMAKE_CURRENT_BINARY_DIR}/scene_${stage}.inc")
    add_custom_command(OUTPUT "${header}" "${binary}"
            COMMAND "${Easy3D_GLSLANG_VALIDATOR}" -V --target-env vulkan1.2 -o "${binary}" "${source}"
            COMMAND "${CMAKE_COMMAND}" "-DINPUT=${binary}" "-DOUTPUT=${header}"
                    "-DSYMBOL=scene_${stage}" -P "${Easy3D_ROOT}/cmake/EmbedSpirv.cmake"
            DEPENDS "${source}" "${Easy3D_ROOT}/cmake/EmbedSpirv.cmake"
            VERBATIM)
    list(APPEND shader_headers "${header}")
endforeach()

add_module(vulkan "render_device.h" "render_device.cpp;vulkan/device.cpp;${shader_headers}"
        "Vulkan::Vulkan" "easy3d::core")
target_include_directories(easy3d_vulkan PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
install(TARGETS easy3d_vulkan EXPORT Easy3DTargets
        RUNTIME DESTINATION "${Easy3D_INSTALL_BIN_DIR}"
        LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}"
        ARCHIVE DESTINATION "${Easy3D_INSTALL_LIB_DIR}")
install(FILES render_device.h DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/easy3d/graphics")

if(Easy3D_BUILD_VULKAN_TESTS)
    add_executable(easy3d_vulkan_test tests/render_test.cpp)
    target_link_libraries(easy3d_vulkan_test PRIVATE easy3d::vulkan)
    set(validation_args)
    if(Easy3D_VULKAN_TEST_VALIDATION)
        set(validation_args --validation)
    endif()
    add_test(NAME vulkan.render COMMAND easy3d_vulkan_test ${validation_args})
    set_tests_properties(vulkan.render PROPERTIES TIMEOUT 120)
endif()
