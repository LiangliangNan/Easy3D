# The root option is the only test switch. Validation is required for Vulkan tests.
set(validation_args --validation)

# The Vulkan loader only finds explicit layers through VK_LAYER_PATH/
# VK_ADD_LAYER_PATH or the platform's registered layer directories. IDEs may
# launch tests without the Vulkan SDK setup script, so locate the manifest at
# configure time and pass its directory to the tests. Missing layers still fail
# the validation tests instead of silently disabling validation.
set(Easy3D_VALIDATION_LAYER_DIR "" CACHE PATH
    "Directory containing VkLayer_khronos_validation.json (auto-detected)")
if(Easy3D_ENABLE_VULKAN AND NOT Easy3D_VALIDATION_LAYER_DIR)
    set(layer_candidates)
    if(DEFINED ENV{VK_LAYER_PATH})
        list(APPEND layer_candidates "$ENV{VK_LAYER_PATH}")
    endif()
    if(DEFINED ENV{VULKAN_SDK})
        list(APPEND layer_candidates
            "$ENV{VULKAN_SDK}/share/vulkan/explicit_layer.d"
            "$ENV{VULKAN_SDK}/Bin")
    endif()
    if(Vulkan_GLSLC_EXECUTABLE)
        get_filename_component(vulkan_sdk_bin "${Vulkan_GLSLC_EXECUTABLE}" DIRECTORY)
        list(APPEND layer_candidates "${vulkan_sdk_bin}/../share/vulkan/explicit_layer.d")
    endif()
    list(APPEND layer_candidates
        "/usr/local/share/vulkan/explicit_layer.d"
        "/usr/share/vulkan/explicit_layer.d"
        "/etc/vulkan/explicit_layer.d"
        "$ENV{HOME}/.local/share/vulkan/explicit_layer.d")
    foreach(candidate IN LISTS layer_candidates)
        if(EXISTS "${candidate}/VkLayer_khronos_validation.json")
            get_filename_component(candidate "${candidate}" ABSOLUTE)
            set(Easy3D_VALIDATION_LAYER_DIR "${candidate}" CACHE PATH
                "Directory containing VkLayer_khronos_validation.json (auto-detected)" FORCE)
            break()
        endif()
    endforeach()
endif()

add_subdirectory(renderer)
add_subdirectory(viewer)
add_subdirectory(gui)
add_subdirectory(core)
add_subdirectory(algo)
add_subdirectory(util)
add_subdirectory(kdtree)
add_subdirectory(python)
include(native_tutorials/test_sources.cmake)
if(TARGET VulkanViewer)
    add_test(NAME vulkan.demo COMMAND VulkanViewer --frames 2 --samples 4 --validation)
    set_tests_properties(vulkan.demo PROPERTIES LABELS desktop TIMEOUT 120)
endif()

include(properties.cmake)
