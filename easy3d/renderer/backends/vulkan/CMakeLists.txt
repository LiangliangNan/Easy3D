find_package(Vulkan REQUIRED)
find_program(Easy3D_GLSLANG_VALIDATOR NAMES glslangValidator
        HINTS "$ENV{VULKAN_SDK}/bin" "$ENV{VULKAN_SDK}/Bin")
if(NOT Easy3D_GLSLANG_VALIDATOR)
    message(FATAL_ERROR "The Vulkan backend requires glslangValidator at build time (Vulkan SDK or glslang-tools). Installed applications do not require it.")
endif()

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/backends/vulkan")
set(vulkan_shader_headers)
foreach(shader scene.vert scene.frag points.vert points.frag points_plain.frag lines.vert lines.frag wide_lines.vert wide_lines.frag overlay.vert overlay.frag hud.frag depth_texture.frag edl.frag ssao.frag ao_gtao.comp ao_blur.comp shadow.vert shadow.frag shadow_depth.frag point_selection.comp oit_quad.vert oit_average.frag oit_peel.frag oit_average_composite.frag oit_peel_composite.frag oit_linked_lists_build.frag oit_linked_lists_resolve.frag)
    string(REPLACE "." "_" symbol "${shader}")
    set(source "${Easy3D_ROOT}/resources/shaders/vulkan/${shader}")
    set(binary "${CMAKE_CURRENT_BINARY_DIR}/backends/vulkan/${symbol}.spv")
    set(header "${CMAKE_CURRENT_BINARY_DIR}/backends/vulkan/${symbol}.inc")
    add_custom_command(OUTPUT "${header}" "${binary}"
            COMMAND "${Easy3D_GLSLANG_VALIDATOR}" -V --target-env vulkan1.2 -o "${binary}" "${source}"
            COMMAND "${CMAKE_COMMAND}" "-DINPUT=${binary}" "-DOUTPUT=${header}"
                    "-DSYMBOL=${symbol}" -P "${Easy3D_ROOT}/cmake/EmbedSpirv.cmake"
            DEPENDS "${source}" "${Easy3D_ROOT}/resources/shaders/vulkan/material.glsl" "${Easy3D_ROOT}/cmake/EmbedSpirv.cmake"
            VERBATIM)
    list(APPEND vulkan_shader_headers "${header}")
endforeach()

list(APPEND backend_sources backends/clipping.cpp
    backends/vulkan/render_device.cpp backends/vulkan/loader.cpp ${vulkan_shader_headers})
list(APPEND private_dependencies ${CMAKE_DL_LIBS})
