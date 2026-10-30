# Build bounded smoke-test entry points from the interactive tutorial scenes.
# Generated copies live in the build tree; tutorials have no test dependencies.
include_guard(GLOBAL)

function(native_tutorial_test_source tutorial output)
    set(directory "${Easy3D_ROOT}/tutorials/${tutorial}")
    set(source "${directory}/main.cpp")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${source}")
    file(READ "${source}" scene)
    string(FIND "${scene}" "return viewer.run();" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Tutorial smoke test needs a viewer.run() entry point: ${tutorial}")
    endif()
    if(NOT tutorial STREQUAL "Tutorial_404_VirtualScanner")
        if(NOT scene MATCHES "viewer\\(EXAMPLE_TITLE[^;\n]*\\);")
            message(FATAL_ERROR "Tutorial smoke test needs a viewer constructor: ${tutorial}")
        endif()
        string(REGEX REPLACE "(viewer\\(EXAMPLE_TITLE[^;\n]*)\\);"
            "\\1, easy3d::test::native_options().viewer);" scene "${scene}")
    endif()
    string(REPLACE "return viewer.run();" "return easy3d::test::native_options().run(viewer);" scene "${scene}")
    set(generated "${CMAKE_CURRENT_BINARY_DIR}/${tutorial}_test.cpp")
    file(GENERATE OUTPUT "${generated}" CONTENT "#include \"${Easy3D_ROOT}/tests/native_tutorials/options.h\"\n#define main tutorial_main\n${scene}\n#undef main\nint main(int argc, char** argv) {\n    if (!easy3d::test::native_options().parse(argc, argv)) return EXIT_FAILURE;\n    return tutorial_main(argc, argv);\n}\n")
    set(${output} "${generated}" PARENT_SCOPE)
endfunction()

# The scanner intentionally exposes only a title constructor. Inject test device
# configuration into its base initializer in a build-tree copy, also used by the
# focused scanner regression. Its public tutorial API stays unchanged.
function(native_scanner_test_source output)
    set(source "${Easy3D_ROOT}/tutorials/Tutorial_404_VirtualScanner/viewer.cpp")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${source}")
    file(READ "${source}" scene)
    set(initializer "VulkanViewer(title)")
    string(FIND "${scene}" "${initializer}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Scanner test adapter needs the tutorial's Vulkan base initializer")
    endif()
    string(REPLACE "${initializer}" "VulkanViewer(title, easy3d::test::native_options().viewer)" scene "${scene}")
    set(generated "${CMAKE_CURRENT_BINARY_DIR}/scanner_test_viewer.cpp")
    file(GENERATE OUTPUT "${generated}" CONTENT "#include \"${Easy3D_ROOT}/tests/native_tutorials/options.h\"\n${scene}")
    set(${output} "${generated}" PARENT_SCOPE)
endfunction()
