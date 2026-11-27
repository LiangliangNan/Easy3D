# Portable value types, options forwarding, and command factories need no device.
add_executable(easy3d_render_device_contract_test render_device_contract_test.cpp)
target_link_libraries(easy3d_render_device_contract_test PRIVATE easy3d::renderer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.devicecontract COMMAND easy3d_render_device_contract_test)
    set_tests_properties(vulkan.devicecontract PROPERTIES TIMEOUT 30)
else()
    add_test(NAME opengl.devicecontract COMMAND easy3d_render_device_contract_test)
    set_tests_properties(opengl.devicecontract PROPERTIES TIMEOUT 30)
endif()

# Declarative effect values and their validation are CPU-only.
add_executable(easy3d_render_settings_test render_settings_test.cpp)
target_link_libraries(easy3d_render_settings_test PRIVATE easy3d::renderer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.rendersettings COMMAND easy3d_render_settings_test)
    set_tests_properties(vulkan.rendersettings PROPERTIES TIMEOUT 30)
else()
    add_test(NAME opengl.rendersettings COMMAND easy3d_render_settings_test)
    set_tests_properties(opengl.rendersettings PROPERTIES TIMEOUT 30)
endif()

# Drawable realization and command styling are CPU-only and run unchanged in both builds.
add_executable(easy3d_drawable_contract_test drawable_contract_test.cpp)
target_link_libraries(easy3d_drawable_contract_test PRIVATE easy3d::renderer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.drawablecontract COMMAND easy3d_drawable_contract_test)
    set_tests_properties(vulkan.drawablecontract PROPERTIES TIMEOUT 30)
else()
    add_test(NAME opengl.drawablecontract COMMAND easy3d_drawable_contract_test)
    set_tests_properties(opengl.drawablecontract PROPERTIES TIMEOUT 30)
endif()

# The camera path builders are CPU geometry: no context or backend device, so this
# test runs unchanged in both builds.
add_executable(easy3d_camera_path_test
        camera_path_test.cpp
        ../../applications/Mapple/camera_path.cpp)
target_include_directories(easy3d_camera_path_test PRIVATE ../../applications/Mapple)
target_link_libraries(easy3d_camera_path_test PRIVATE easy3d::renderer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.camerapathgeneration COMMAND easy3d_camera_path_test)
    set_tests_properties(vulkan.camerapathgeneration PROPERTIES TIMEOUT 120)
else()
    add_test(NAME opengl.camerapathgeneration COMMAND easy3d_camera_path_test)
    set_tests_properties(opengl.camerapathgeneration PROPERTIES TIMEOUT 120)
endif()

# The interpolator's timing (per-keyframe dwell, speed profiles) is CPU arithmetic too.
add_executable(easy3d_keyframe_interpolator_test keyframe_interpolator_test.cpp)
target_link_libraries(easy3d_keyframe_interpolator_test PRIVATE easy3d::renderer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.keyframeinterpolator COMMAND easy3d_keyframe_interpolator_test)
    set_tests_properties(vulkan.keyframeinterpolator PROPERTIES TIMEOUT 120)
else()
    add_test(NAME opengl.keyframeinterpolator COMMAND easy3d_keyframe_interpolator_test)
    set_tests_properties(opengl.keyframeinterpolator PROPERTIES TIMEOUT 120)
endif()

# Mapple's .kf composition adds generated-source provenance to the library stream.
add_executable(easy3d_camera_path_io_test
        camera_path_io_test.cpp
        ../../applications/Mapple/camera_path_io.cpp
        ../../applications/Mapple/camera_path.cpp)
target_include_directories(easy3d_camera_path_io_test PRIVATE ../../applications/Mapple)
target_link_libraries(easy3d_camera_path_io_test PRIVATE easy3d::renderer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.camerapathio COMMAND easy3d_camera_path_io_test)
    set_tests_properties(vulkan.camerapathio PROPERTIES TIMEOUT 120)
else()
    add_test(NAME opengl.camerapathio COMMAND easy3d_camera_path_io_test)
    set_tests_properties(opengl.camerapathio PROPERTIES TIMEOUT 120)
endif()

# The editor's insert/delete rebuild the path, which is CPU bookkeeping as well: the
# poses, times and dwell pauses either survive the rebuild or they do not.
add_executable(easy3d_keyframe_editor_test
        keyframe_editor_test.cpp
        ../../applications/Mapple/keyframe_editor.cpp
        ../../applications/Mapple/keyframe_gizmo.cpp)
target_include_directories(easy3d_keyframe_editor_test PRIVATE ../../applications/Mapple)
target_link_libraries(easy3d_keyframe_editor_test PRIVATE easy3d::renderer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.keyframeeditor COMMAND easy3d_keyframe_editor_test)
    set_tests_properties(vulkan.keyframeeditor PROPERTIES TIMEOUT 120)
else()
    add_test(NAME opengl.keyframeeditor COMMAND easy3d_keyframe_editor_test)
    set_tests_properties(opengl.keyframeeditor PROPERTIES TIMEOUT 120)
endif()

if(Easy3D_ENABLE_VULKAN)
    add_executable(easy3d_vulkan_test render_test.cpp)
    target_link_libraries(easy3d_vulkan_test PRIVATE easy3d::renderer)
    add_executable(easy3d_vulkan_resource_lifetime_test resource_lifetime_test.cpp)
    target_link_libraries(easy3d_vulkan_resource_lifetime_test PRIVATE easy3d::renderer)
    add_test(NAME vulkan.resourcelifetime COMMAND easy3d_vulkan_resource_lifetime_test ${validation_args})
    set_tests_properties(vulkan.resourcelifetime PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_text_test text_test.cpp)
    # text_test.cpp checks a UTF-8 fixture and its literals are UTF-8 only when
    # MSVC reads the source as UTF-8; the default ANSI codepage silently
    # double-encodes them.
    target_compile_options(easy3d_vulkan_text_test PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
    target_link_libraries(easy3d_vulkan_text_test PRIVATE easy3d::renderer easy3d::util)
    add_test(NAME vulkan.text COMMAND easy3d_vulkan_text_test ${validation_args})
    set_tests_properties(vulkan.text PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_depth_overlay_test depth_overlay_test.cpp)
    target_link_libraries(easy3d_vulkan_depth_overlay_test PRIVATE easy3d::renderer)
    add_test(NAME vulkan.depthoverlay COMMAND easy3d_vulkan_depth_overlay_test ${validation_args})
    set_tests_properties(vulkan.depthoverlay PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_depth_texture_test depth_texture_test.cpp)
    target_link_libraries(easy3d_vulkan_depth_texture_test PRIVATE easy3d::renderer)
    add_test(NAME vulkan.depthtexture COMMAND easy3d_vulkan_depth_texture_test ${validation_args})
    set_tests_properties(vulkan.depthtexture PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_point_selection_test point_selection_test.cpp)
    target_link_libraries(easy3d_vulkan_point_selection_test PRIVATE easy3d::renderer)
    add_test(NAME vulkan.pointselection COMMAND easy3d_vulkan_point_selection_test ${validation_args})
    set_tests_properties(vulkan.pointselection PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_eye_dome_lighting_test eye_dome_lighting_test.cpp)
    target_link_libraries(easy3d_vulkan_eye_dome_lighting_test PRIVATE easy3d::renderer)
    add_test(NAME vulkan.eyedomelighting COMMAND easy3d_vulkan_eye_dome_lighting_test ${validation_args})
    set_tests_properties(vulkan.eyedomelighting PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_ambient_occlusion_test ambient_occlusion_test.cpp)
    target_link_libraries(easy3d_vulkan_ambient_occlusion_test PRIVATE easy3d::renderer)
    add_test(NAME vulkan.ambientocclusion COMMAND easy3d_vulkan_ambient_occlusion_test ${validation_args})
    set_tests_properties(vulkan.ambientocclusion PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_transparency_test transparency_test.cpp)
    target_link_libraries(easy3d_vulkan_transparency_test PRIVATE easy3d::renderer)
    add_test(NAME vulkan.transparency COMMAND easy3d_vulkan_transparency_test ${validation_args})
    set_tests_properties(vulkan.transparency PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_shadow_test shadow_test.cpp)
    target_link_libraries(easy3d_vulkan_shadow_test PRIVATE easy3d::renderer)
    add_test(NAME vulkan.shadow COMMAND easy3d_vulkan_shadow_test ${validation_args})
    set_tests_properties(vulkan.shadow PROPERTIES TIMEOUT 120)
    add_test(NAME vulkan.render COMMAND easy3d_vulkan_test ${validation_args})
    set_tests_properties(vulkan.render PROPERTIES TIMEOUT 120)
else()
    add_executable(easy3d_opengl_render_test render_test.cpp)
    target_compile_definitions(easy3d_opengl_render_test PRIVATE EASY3D_TEST_OPENGL)
    target_link_libraries(easy3d_opengl_render_test PRIVATE easy3d::renderer easy3d::viewer)
    add_test(NAME opengl.render COMMAND easy3d_opengl_render_test)
    set_tests_properties(opengl.render PROPERTIES TIMEOUT 120)
endif()

include(../properties.cmake)
