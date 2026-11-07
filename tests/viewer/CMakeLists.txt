# Reuse tutorial implementations without test guards in production sources.
foreach(tutorial IN ITEMS Tutorial_601_Tessellator Tutorial_602_ConvexPartition Tutorial_603_Curves
        Tutorial_701_Cloud_NormalEstimation Tutorial_702_Cloud_SurfaceReconstruction Tutorial_703_Cloud_PlaneExtraction)
    set_source_files_properties("../../tutorials/${tutorial}/main.cpp" PROPERTIES
        COMPILE_DEFINITIONS "main=easy3d_test_${tutorial}_main;EXAMPLE_TITLE=\"${tutorial}\"")
endforeach()

    add_executable(easy3d_multi_viewer_test multi_viewer_test.cpp ../../applications/FigureMaker/viewer.cpp)
    target_link_libraries(easy3d_multi_viewer_test PRIVATE easy3d::viewer 3rd_glfw)
    if(Easy3D_ENABLE_VULKAN)
        add_test(NAME vulkan.multiview COMMAND easy3d_multi_viewer_test --validation)
        set_tests_properties(vulkan.multiview PROPERTIES TIMEOUT 120 LABELS desktop)
    else()
        add_test(NAME opengl.multiview COMMAND easy3d_multi_viewer_test)
        set_tests_properties(opengl.multiview PROPERTIES TIMEOUT 120 LABELS desktop)
    endif()

    add_executable(easy3d_offscreen_backend_test offscreen_backend_test.cpp)
    target_link_libraries(easy3d_offscreen_backend_test PRIVATE easy3d::viewer)
    add_test(NAME viewer.offscreen COMMAND easy3d_offscreen_backend_test)
    set_tests_properties(viewer.offscreen PROPERTIES TIMEOUT 120)
    if(NOT Easy3D_ENABLE_VULKAN)
        set_tests_properties(viewer.offscreen PROPERTIES LABELS desktop)
    endif()

    add_executable(easy3d_viewer_query_test viewer_query_test.cpp)
    target_link_libraries(easy3d_viewer_query_test PRIVATE easy3d::viewer 3rd_glfw)
    add_executable(easy3d_viewer_callbacks_test viewer_callbacks_test.cpp)
    target_link_libraries(easy3d_viewer_callbacks_test PRIVATE easy3d::viewer 3rd_glfw)
    if(NOT Easy3D_ENABLE_VULKAN)
        add_test(NAME opengl.query COMMAND easy3d_viewer_query_test)
        set_tests_properties(opengl.query PROPERTIES TIMEOUT 120 LABELS desktop)
        add_test(NAME opengl.callbacks COMMAND easy3d_viewer_callbacks_test)
        set_tests_properties(opengl.callbacks PROPERTIES TIMEOUT 120 LABELS desktop)
    endif()
    if(Easy3D_ENABLE_VULKAN)
        add_test(NAME vulkan.query COMMAND easy3d_viewer_query_test ${validation_args})
        set_tests_properties(vulkan.query PROPERTIES TIMEOUT 120 LABELS desktop)
        add_test(NAME vulkan.callbacks COMMAND easy3d_viewer_callbacks_test ${validation_args})
        set_tests_properties(vulkan.callbacks PROPERTIES TIMEOUT 120 LABELS desktop)
    endif()

if(Easy3D_ENABLE_VULKAN)
    add_executable(easy3d_vulkan_scene_test scene_test.cpp)
    target_link_libraries(easy3d_vulkan_scene_test PRIVATE easy3d::viewer)
    add_test(NAME vulkan.scene COMMAND easy3d_vulkan_scene_test ${validation_args})
    set_tests_properties(vulkan.scene PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_scan_test scan_test.cpp)
    target_link_libraries(easy3d_vulkan_scan_test PRIVATE easy3d::viewer)
    add_test(NAME vulkan.scan COMMAND easy3d_vulkan_scan_test ${validation_args})
    set_tests_properties(vulkan.scan PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_clipping_test clipping_test.cpp)
    target_link_libraries(easy3d_vulkan_clipping_test PRIVATE easy3d::viewer)
    add_test(NAME vulkan.clipping COMMAND easy3d_vulkan_clipping_test ${validation_args})
    set_tests_properties(vulkan.clipping PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_picking_test picking_test.cpp)
    target_link_libraries(easy3d_vulkan_picking_test PRIVATE easy3d::viewer)
    add_test(NAME vulkan.picking COMMAND easy3d_vulkan_picking_test ${validation_args})
    set_tests_properties(vulkan.picking PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_obj_material_test obj_material_test.cpp)
    target_link_libraries(easy3d_vulkan_obj_material_test PRIVATE easy3d::viewer)
    add_test(NAME vulkan.objmaterials COMMAND easy3d_vulkan_obj_material_test ${validation_args})
    set_tests_properties(vulkan.objmaterials PROPERTIES TIMEOUT 120)
    add_executable(easy3d_vulkan_texture_test texture_test.cpp)
    target_link_libraries(easy3d_vulkan_texture_test PRIVATE easy3d::viewer)
    add_test(NAME vulkan.texture COMMAND easy3d_vulkan_texture_test ${validation_args})
    set_tests_properties(vulkan.texture PROPERTIES TIMEOUT 120)
    get_target_property(algo_definitions easy3d_algo COMPILE_DEFINITIONS)
    set(cloud_tutorial_definitions)
    if("HAS_BOOST" IN_LIST algo_definitions)
        list(APPEND cloud_tutorial_definitions EASY3D_TEST_HAS_BOOST)
    endif()
    set(cloud_tutorial_sources cloud_tutorials_test.cpp
        ../../tutorials/Tutorial_701_Cloud_NormalEstimation/main.cpp
        ../../tutorials/Tutorial_702_Cloud_SurfaceReconstruction/main.cpp
        ../../tutorials/Tutorial_703_Cloud_PlaneExtraction/main.cpp)
    if(Easy3D_ENABLE_VULKAN)
        add_executable(easy3d_vulkan_viewer_loop_test viewer_loop_test.cpp)
        target_link_libraries(easy3d_vulkan_viewer_loop_test PRIVATE easy3d::viewer 3rd_glfw)
        add_test(NAME vulkan.viewerloop COMMAND easy3d_vulkan_viewer_loop_test ${validation_args})
        set_tests_properties(vulkan.viewerloop PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_multithread_test multithread_test.cpp)
        target_link_libraries(easy3d_vulkan_multithread_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.multithread COMMAND easy3d_vulkan_multithread_test ${validation_args})
        set_tests_properties(vulkan.multithread PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_animation_test animation_test.cpp)
        target_link_libraries(easy3d_vulkan_animation_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.animation COMMAND easy3d_vulkan_animation_test ${validation_args})
        set_tests_properties(vulkan.animation PROPERTIES TIMEOUT 120 LABELS desktop)

        add_executable(easy3d_vulkan_scan_viewer_test scan_viewer_test.cpp
                "${Easy3D_ROOT}/tutorials/Tutorial_404_VirtualScanner/viewer.cpp")
        target_include_directories(easy3d_vulkan_scan_viewer_test PRIVATE
                "${Easy3D_ROOT}/tutorials/Tutorial_404_VirtualScanner")
        target_link_libraries(easy3d_vulkan_scan_viewer_test PRIVATE easy3d::viewer)

        add_test(NAME vulkan.scanviewer COMMAND easy3d_vulkan_scan_viewer_test ${validation_args})
        set_tests_properties(vulkan.scanviewer PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_point_selection_viewer_test point_selection_viewer_test.cpp
                ../../tutorials/Tutorial_403_PointSelection/viewer.cpp)
        target_link_libraries(easy3d_vulkan_point_selection_viewer_test PRIVATE easy3d::viewer)

        add_test(NAME vulkan.pointselectionviewer COMMAND easy3d_vulkan_point_selection_viewer_test ${validation_args})
        set_tests_properties(vulkan.pointselectionviewer PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_cross_section_test cross_section_test.cpp
                ../../tutorials/Tutorial_307_CrossSection/viewer.cpp)
        target_link_libraries(easy3d_vulkan_cross_section_test PRIVATE easy3d::viewer)

        add_test(NAME vulkan.crosssection COMMAND easy3d_vulkan_cross_section_test ${validation_args})
        set_tests_properties(vulkan.crosssection PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_picker_viewer_test picker_viewer_test.cpp
                ../../tutorials/Tutorial_401_ModelPicker/viewer.cpp ../../tutorials/Tutorial_402_FacePicker/viewer.cpp)
        target_link_libraries(easy3d_vulkan_picker_viewer_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.pickerviewer COMMAND easy3d_vulkan_picker_viewer_test ${validation_args})
        set_tests_properties(vulkan.pickerviewer PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_obj_viewer_test obj_viewer_test.cpp
                ../../tutorials/Tutorial_308_TexturedMesh/viewer.cpp)
        target_link_libraries(easy3d_vulkan_obj_viewer_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.objviewer COMMAND easy3d_vulkan_obj_viewer_test ${validation_args})
        set_tests_properties(vulkan.objviewer PROPERTIES TIMEOUT 120 LABELS desktop)

        add_executable(easy3d_vulkan_frame_rate_test frame_rate_test.cpp)
        target_link_libraries(easy3d_vulkan_frame_rate_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.framerate COMMAND easy3d_vulkan_frame_rate_test ${validation_args})
        set_tests_properties(vulkan.framerate PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_snapshot_effects_test snapshot_effects_test.cpp)
        target_link_libraries(easy3d_vulkan_snapshot_effects_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.snapshoteffects COMMAND easy3d_vulkan_snapshot_effects_test ${validation_args})
        set_tests_properties(vulkan.snapshoteffects PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_clipping_plane_test clipping_plane_test.cpp)
        target_link_libraries(easy3d_vulkan_clipping_plane_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.clippingplane COMMAND easy3d_vulkan_clipping_plane_test ${validation_args})
        set_tests_properties(vulkan.clippingplane PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_corner_axes_test corner_axes_test.cpp)
        target_link_libraries(easy3d_vulkan_corner_axes_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.corneraxes COMMAND easy3d_vulkan_corner_axes_test ${validation_args})
        set_tests_properties(vulkan.corneraxes PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_real_camera_test real_camera_test.cpp
                ../../tutorials/Tutorial_207_RealCamera/viewer.cpp
                ../../tutorials/Tutorial_207_RealCamera/read_bundler_file.cpp)
        target_link_libraries(easy3d_vulkan_real_camera_test PRIVATE easy3d::viewer)

        add_test(NAME vulkan.realcamera COMMAND easy3d_vulkan_real_camera_test ${validation_args})
        set_tests_properties(vulkan.realcamera PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_depth_map_test depth_map_test.cpp
                ../../tutorials/Tutorial_506_DepthMap/viewer.cpp)
        target_link_libraries(easy3d_vulkan_depth_map_test PRIVATE easy3d::viewer)

        add_test(NAME vulkan.depthmap COMMAND easy3d_vulkan_depth_map_test ${validation_args})
        set_tests_properties(vulkan.depthmap PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_screen_lines_test screen_lines_test.cpp)
        target_link_libraries(easy3d_vulkan_screen_lines_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.screenlines COMMAND easy3d_vulkan_screen_lines_test ${validation_args})
        set_tests_properties(vulkan.screenlines PROPERTIES TIMEOUT 120)
        add_executable(easy3d_vulkan_cloud_tutorials_test ${cloud_tutorial_sources})
        target_compile_definitions(easy3d_vulkan_cloud_tutorials_test PRIVATE ${cloud_tutorial_definitions})
        target_link_libraries(easy3d_vulkan_cloud_tutorials_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.cloudtutorials COMMAND easy3d_vulkan_cloud_tutorials_test ${validation_args})
        set_tests_properties(vulkan.cloudtutorials PROPERTIES TIMEOUT 180 LABELS desktop)
        add_executable(easy3d_vulkan_geometry_tutorials_test geometry_tutorials_test.cpp
                ../../tutorials/Tutorial_601_Tessellator/main.cpp
                ../../tutorials/Tutorial_602_ConvexPartition/main.cpp
                ../../tutorials/Tutorial_603_Curves/main.cpp)
        target_link_libraries(easy3d_vulkan_geometry_tutorials_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.geometrytutorials COMMAND easy3d_vulkan_geometry_tutorials_test ${validation_args})
        set_tests_properties(vulkan.geometrytutorials PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_viewer_camera_path_test viewer_camera_path_test.cpp)
        target_link_libraries(easy3d_vulkan_viewer_camera_path_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.viewercamerapath COMMAND easy3d_vulkan_viewer_camera_path_test ${validation_args})
        set_tests_properties(vulkan.viewercamerapath PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_camera_path_test camera_path_test.cpp
                ../../tutorials/Tutorial_206_CameraInterpolation/viewer.cpp)
        target_link_libraries(easy3d_vulkan_camera_path_test PRIVATE easy3d::viewer)

        add_test(NAME vulkan.camerapath COMMAND easy3d_vulkan_camera_path_test ${validation_args})
        set_tests_properties(vulkan.camerapath PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_collision_test collision_test.cpp
                ../../tutorials/Tutorial_406_CollisionDetection/viewer.cpp)
        target_link_libraries(easy3d_vulkan_collision_test PRIVATE easy3d::viewer)

        add_test(NAME vulkan.collision COMMAND easy3d_vulkan_collision_test ${validation_args})
        set_tests_properties(vulkan.collision PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_manipulation_test manipulation_test.cpp
                ../../tutorials/Tutorial_405_ObjectManipulation/viewer.cpp)
        target_link_libraries(easy3d_vulkan_manipulation_test PRIVATE easy3d::viewer)

        add_test(NAME vulkan.manipulation COMMAND easy3d_vulkan_manipulation_test ${validation_args})
        set_tests_properties(vulkan.manipulation PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_usage_test usage_test.cpp
                ../../tutorials/Tutorial_306_ImageViewer/viewer.cpp
                ../../tutorials/Tutorial_307_CrossSection/viewer.cpp
                ../../tutorials/Tutorial_309_TextRendering/viewer.cpp
                ../../tutorials/Tutorial_401_ModelPicker/viewer.cpp
                ../../tutorials/Tutorial_402_FacePicker/viewer.cpp
                ../../tutorials/Tutorial_403_PointSelection/viewer.cpp
                ../../tutorials/Tutorial_404_VirtualScanner/viewer.cpp)
        target_include_directories(easy3d_vulkan_usage_test PRIVATE
                "${Easy3D_ROOT}/tutorials/Tutorial_404_VirtualScanner")
        target_link_libraries(easy3d_vulkan_usage_test PRIVATE easy3d::viewer)

        add_test(NAME vulkan.usage COMMAND easy3d_vulkan_usage_test ${validation_args})
        set_tests_properties(vulkan.usage PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_interaction_overlay_test interaction_overlay_test.cpp)
        target_link_libraries(easy3d_vulkan_interaction_overlay_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.interactionoverlay COMMAND easy3d_vulkan_interaction_overlay_test ${validation_args})
        set_tests_properties(vulkan.interactionoverlay PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_input_test input_test.cpp)
        target_link_libraries(easy3d_vulkan_input_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.input COMMAND easy3d_vulkan_input_test ${validation_args})
        set_tests_properties(vulkan.input PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_window_test window_test.cpp)
        target_link_libraries(easy3d_vulkan_window_test PRIVATE easy3d::viewer)
        add_test(NAME vulkan.window COMMAND easy3d_vulkan_window_test ${validation_args})
        set_tests_properties(vulkan.window PROPERTIES TIMEOUT 120 LABELS "desktop")
        add_executable(easy3d_vulkan_text_viewer_test text_viewer_test.cpp
                    ../../tutorials/Tutorial_309_TextRendering/viewer.cpp)
        target_link_libraries(easy3d_vulkan_text_viewer_test PRIVATE easy3d::viewer)

        add_test(NAME vulkan.textviewer COMMAND easy3d_vulkan_text_viewer_test ${validation_args})
        set_tests_properties(vulkan.textviewer PROPERTIES TIMEOUT 120 LABELS desktop)
        add_executable(easy3d_vulkan_image_viewer_test image_viewer_test.cpp
                    ../../tutorials/Tutorial_306_ImageViewer/viewer.cpp)
        target_link_libraries(easy3d_vulkan_image_viewer_test PRIVATE easy3d::viewer)

        add_test(NAME vulkan.imageviewer COMMAND easy3d_vulkan_image_viewer_test ${validation_args})
        set_tests_properties(vulkan.imageviewer PROPERTIES TIMEOUT 120 LABELS desktop)
    endif()
endif()

if(NOT Easy3D_ENABLE_VULKAN)
    get_target_property(algo_definitions easy3d_algo COMPILE_DEFINITIONS)
    set(cloud_tutorial_definitions)
    if("HAS_BOOST" IN_LIST algo_definitions)
        list(APPEND cloud_tutorial_definitions EASY3D_TEST_HAS_BOOST)
    endif()
    set(cloud_tutorial_sources cloud_tutorials_test.cpp
        ../../tutorials/Tutorial_701_Cloud_NormalEstimation/main.cpp
        ../../tutorials/Tutorial_702_Cloud_SurfaceReconstruction/main.cpp
        ../../tutorials/Tutorial_703_Cloud_PlaneExtraction/main.cpp)

    add_executable(easy3d_opengl_viewer_loop_test viewer_loop_test.cpp)
    target_compile_definitions(easy3d_opengl_viewer_loop_test PRIVATE EASY3D_TEST_OPENGL_VIEWER)
    target_link_libraries(easy3d_opengl_viewer_loop_test PRIVATE easy3d::viewer 3rd_glfw)
    add_test(NAME opengl.viewerloop COMMAND easy3d_opengl_viewer_loop_test)
    set_tests_properties(opengl.viewerloop PROPERTIES TIMEOUT 120 LABELS desktop)
    add_executable(easy3d_opengl_obj_viewer_test obj_viewer_test.cpp
            ../../tutorials/Tutorial_308_TexturedMesh/viewer.cpp)
    target_link_libraries(easy3d_opengl_obj_viewer_test PRIVATE easy3d::viewer)
    add_test(NAME opengl.objmaterials COMMAND easy3d_opengl_obj_viewer_test)
    set_tests_properties(opengl.objmaterials PROPERTIES TIMEOUT 120 LABELS desktop)
    add_executable(easy3d_opengl_screen_lines_test opengl_screen_lines_test.cpp
        ../../tutorials/Tutorial_603_Curves/main.cpp)
    target_link_libraries(easy3d_opengl_screen_lines_test PRIVATE easy3d::viewer)
    add_test(NAME opengl.screenlines COMMAND easy3d_opengl_screen_lines_test)
    set_tests_properties(opengl.screenlines PROPERTIES TIMEOUT 120 LABELS desktop)
    add_executable(easy3d_opengl_cloud_tutorials_test ${cloud_tutorial_sources})
    target_compile_definitions(easy3d_opengl_cloud_tutorials_test PRIVATE ${cloud_tutorial_definitions})
    target_link_libraries(easy3d_opengl_cloud_tutorials_test PRIVATE easy3d::viewer)
    add_test(NAME opengl.cloudtutorials COMMAND easy3d_opengl_cloud_tutorials_test)
    set_tests_properties(opengl.cloudtutorials PROPERTIES TIMEOUT 180 LABELS desktop)
    add_executable(easy3d_opengl_texture_test texture_gl_test.cpp)
    target_link_libraries(easy3d_opengl_texture_test PRIVATE easy3d::viewer)
    add_test(NAME opengl.texture COMMAND easy3d_opengl_texture_test)
    set_tests_properties(opengl.texture PROPERTIES TIMEOUT 120 LABELS desktop)

    add_executable(easy3d_opengl_counterparts_test opengl_counterparts_test.cpp)
    target_link_libraries(easy3d_opengl_counterparts_test PRIVATE easy3d::viewer)
    add_test(NAME opengl.counterparts COMMAND easy3d_opengl_counterparts_test)
    set_tests_properties(opengl.counterparts PROPERTIES TIMEOUT 120 LABELS desktop)

    add_executable(easy3d_opengl_snapshot_seam_test opengl_snapshot_seam_test.cpp)
    target_link_libraries(easy3d_opengl_snapshot_seam_test PRIVATE easy3d::viewer)
    add_test(NAME opengl.snapshotseam COMMAND easy3d_opengl_snapshot_seam_test)
    set_tests_properties(opengl.snapshotseam PROPERTIES TIMEOUT 120 LABELS desktop)

    add_executable(easy3d_opengl_ddp_seam_test opengl_ddp_seam_test.cpp)
    target_link_libraries(easy3d_opengl_ddp_seam_test PRIVATE easy3d::viewer)
    add_test(NAME opengl.ddpseam COMMAND easy3d_opengl_ddp_seam_test)
    set_tests_properties(opengl.ddpseam PROPERTIES TIMEOUT 120 LABELS desktop)
endif()

    add_executable(easy3d_imgui_viewer_test imgui_viewer_test.cpp
        ../../tutorials/Tutorial_202_Viewer_imgui/viewer.cpp)
    target_link_libraries(easy3d_imgui_viewer_test PRIVATE easy3d::viewer 3rd_imgui)
    if(Easy3D_ENABLE_VULKAN)
        add_test(NAME vulkan.imgui COMMAND easy3d_imgui_viewer_test ${validation_args})
        set_tests_properties(vulkan.imgui PROPERTIES TIMEOUT 120 LABELS desktop)
    else()
        add_test(NAME opengl.imgui COMMAND easy3d_imgui_viewer_test)
        set_tests_properties(opengl.imgui PROPERTIES TIMEOUT 120 LABELS desktop)
    endif()

add_executable(easy3d_portable_pick_test portable_pick_test.cpp
    ../../tutorials/Tutorial_401_ModelPicker/viewer.cpp ../../tutorials/Tutorial_402_FacePicker/viewer.cpp
    ../../tutorials/Tutorial_405_ObjectManipulation/viewer.cpp)
target_link_libraries(easy3d_portable_pick_test PRIVATE easy3d::viewer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.portablepick COMMAND easy3d_portable_pick_test --validation)
    set_tests_properties(vulkan.portablepick PROPERTIES LABELS desktop TIMEOUT 120)
else()
    add_test(NAME opengl.portablepick COMMAND easy3d_portable_pick_test)
    set_tests_properties(opengl.portablepick PROPERTIES LABELS desktop TIMEOUT 120)
endif()

add_executable(easy3d_overlay_viewer_test overlay_viewer_test.cpp
    ../../tutorials/Tutorial_306_ImageViewer/viewer.cpp ../../tutorials/Tutorial_309_TextRendering/viewer.cpp)
target_link_libraries(easy3d_overlay_viewer_test PRIVATE easy3d::viewer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.annotations COMMAND easy3d_overlay_viewer_test --validation)
    set_tests_properties(vulkan.annotations PROPERTIES LABELS desktop TIMEOUT 120)
else()
    add_test(NAME opengl.annotations COMMAND easy3d_overlay_viewer_test)
    set_tests_properties(opengl.annotations PROPERTIES LABELS desktop TIMEOUT 120)
endif()

add_executable(easy3d_annotation_tutorials_test annotation_tutorials_test.cpp
    ../../tutorials/Tutorial_207_RealCamera/viewer.cpp
    ../../tutorials/Tutorial_207_RealCamera/read_bundler_file.cpp
    ../../tutorials/Tutorial_405_ObjectManipulation/viewer.cpp
    ../../tutorials/Tutorial_506_DepthMap/viewer.cpp)
target_link_libraries(easy3d_annotation_tutorials_test PRIVATE easy3d::viewer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.annotationtutorials COMMAND easy3d_annotation_tutorials_test)
    set_tests_properties(vulkan.annotationtutorials PROPERTIES LABELS desktop TIMEOUT 180)
else()
    add_test(NAME opengl.annotationtutorials COMMAND easy3d_annotation_tutorials_test)
    set_tests_properties(opengl.annotationtutorials PROPERTIES LABELS desktop TIMEOUT 180)
endif()


add_executable(easy3d_independent_hud_test independent_hud_test.cpp)
target_link_libraries(easy3d_independent_hud_test PRIVATE easy3d::viewer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.independenthud COMMAND easy3d_independent_hud_test --validation)
    set_tests_properties(vulkan.independenthud PROPERTIES LABELS desktop TIMEOUT 120)
else()
    add_test(NAME opengl.independenthud COMMAND easy3d_independent_hud_test)
    set_tests_properties(opengl.independenthud PROPERTIES LABELS desktop TIMEOUT 120)
endif()

add_executable(easy3d_portable_selection_test portable_selection_test.cpp
    ../../tutorials/Tutorial_403_PointSelection/viewer.cpp)
target_link_libraries(easy3d_portable_selection_test PRIVATE easy3d::viewer)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.portableselection COMMAND easy3d_portable_selection_test --validation)
    set_tests_properties(vulkan.portableselection PROPERTIES LABELS desktop TIMEOUT 120)
else()
    add_test(NAME opengl.portableselection COMMAND easy3d_portable_selection_test)
    set_tests_properties(opengl.portableselection PROPERTIES LABELS desktop TIMEOUT 120)
endif()

add_executable(easy3d_offscreen_query_test offscreen_query_test.cpp)
target_link_libraries(easy3d_offscreen_query_test PRIVATE easy3d::viewer 3rd_glfw)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.offscreenqueries COMMAND easy3d_offscreen_query_test --validation)
    set_tests_properties(vulkan.offscreenqueries PROPERTIES LABELS desktop TIMEOUT 120)
else()
    add_test(NAME opengl.offscreenqueries COMMAND easy3d_offscreen_query_test)
    set_tests_properties(opengl.offscreenqueries PROPERTIES LABELS desktop TIMEOUT 120)
endif()

add_executable(easy3d_capture_test capture_test.cpp ../../tutorials/Tutorial_404_VirtualScanner/viewer.cpp)
target_link_libraries(easy3d_capture_test PRIVATE easy3d::viewer 3rd_glfw)
if(Easy3D_ENABLE_VULKAN)
    add_test(NAME vulkan.capture COMMAND easy3d_capture_test --validation)
    set_tests_properties(vulkan.capture PROPERTIES LABELS desktop TIMEOUT 120)
else()
    add_test(NAME opengl.capture COMMAND easy3d_capture_test)
    set_tests_properties(opengl.capture PROPERTIES LABELS desktop TIMEOUT 120)
endif()

include(../properties.cmake)
