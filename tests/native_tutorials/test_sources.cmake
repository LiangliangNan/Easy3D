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
    string(REPLACE "return viewer.run();" "return easy3d::test::native_options().run(viewer);" scene "${scene}")
    set(generated "${CMAKE_CURRENT_BINARY_DIR}/native_tutorials/${tutorial}_test.cpp")
    file(GENERATE OUTPUT "${generated}" CONTENT "#include \"${Easy3D_ROOT}/tests/native_tutorials/options.h\"\n#define main tutorial_main\n${scene}\n#undef main\nint main(int argc, char** argv) {\n    if (!easy3d::test::native_options().parse(argc, argv)) return EXIT_FAILURE;\n    return tutorial_main(argc, argv);\n}\n")
    set(${output} "${generated}" PARENT_SCOPE)
endfunction()

# Keep smoke-test targets independent of the interactive tutorial targets.
# The same original scenes and bounded driver exercise either configured backend.
if(Easy3D_ENABLE_VULKAN)
    set(test_prefix vulkan)
else()
    set(test_prefix opengl)
endif()
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/native_tutorials")
set(tutorial_scenes
        Tutorial_201_Viewer_default
        Tutorial_202_Viewer_imgui
        Tutorial_203_MultiView
        Tutorial_205_RealCamera
        Tutorial_206_CameraInterpolation
        Tutorial_301_Drawables
        Tutorial_302_Imposters
        Tutorial_303_ScalarField
        Tutorial_304_VectorField
        Tutorial_305_Texture
        Tutorial_306_ImageViewer
        Tutorial_307_CrossSection
        Tutorial_308_TexturedMesh
        Tutorial_309_TextRendering
        Tutorial_310_TextMesher
        Tutorial_311_Animation
        Tutorial_312_MultiThread
        Tutorial_401_ModelPicker
        Tutorial_402_FacePicker
        Tutorial_403_PointSelection
        Tutorial_404_VirtualScanner
        Tutorial_405_ObjectManipulation
        Tutorial_406_CollisionDetection
        Tutorial_501_AmbientOcclusion
        Tutorial_502_HardShadow
        Tutorial_503_SoftShadow
        Tutorial_504_Transparency
        Tutorial_505_EyeDomeLighting
        Tutorial_506_DepthMap
        Tutorial_601_Tessellator
        Tutorial_602_ConvexPartition
        Tutorial_603_CSG
        Tutorial_604_Curves
        Tutorial_605_Delaunay2D
        Tutorial_701_Cloud_NormalEstimation
        Tutorial_702_Cloud_SurfaceReconstruction
        Tutorial_703_Cloud_PlaneExtraction
        Tutorial_704_Cloud_DownSampling
        Tutorial_801_Mesh_Curvature
        Tutorial_802_Mesh_Smoothing
        Tutorial_803_Mesh_Fairing
        Tutorial_804_Mesh_HoleFilling
        Tutorial_805_Mesh_Remeshing
        Tutorial_806_Mesh_Simplification
        Tutorial_807_Mesh_Subdivision
        Tutorial_808_Mesh_Parameterization
)
foreach(tutorial IN LISTS tutorial_scenes)
    string(REGEX MATCH "^Tutorial_([0-9]+)_" tutorial_prefix "${tutorial}")
    set(test_name ${test_prefix}.tutorial${CMAKE_MATCH_1})
    set(test_target ${tutorial}_test)
    set(directory "${Easy3D_ROOT}/tutorials/${tutorial}")
    native_tutorial_test_source(${tutorial} test_source)
    add_executable(${test_target} "${test_source}")
    target_include_directories(${test_target} PRIVATE "${directory}")
    if(EXISTS "${directory}/viewer.cpp")
        target_sources(${test_target} PRIVATE "${directory}/viewer.cpp")
    endif()
    if(EXISTS "${directory}/read_bundler_file.cpp")
        target_sources(${test_target} PRIVATE "${directory}/read_bundler_file.cpp")
    endif()
    target_compile_definitions(${test_target} PRIVATE EXAMPLE_TITLE="${tutorial}")
    target_link_libraries(${test_target} PRIVATE easy3d::viewer easy3d::gui)
    if(tutorial STREQUAL "Tutorial_202_Viewer_imgui")
        target_link_libraries(${test_target} PRIVATE 3rd_imgui)
    endif()
    set_target_properties(${test_target} PROPERTIES FOLDER "tests")
    add_test(NAME ${test_name} COMMAND ${test_target} --frames 3)
    set_tests_properties(${test_name} PROPERTIES LABELS desktop TIMEOUT 120
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/native_tutorials")
endforeach()
