function(add_example example files dependencies)
    add_executable(${example} ${files})
    set_target_properties(${example} PROPERTIES FOLDER "tutorials")
    target_include_directories(${example} PRIVATE ${Easy3D_INCLUDE_DIR})
    target_link_libraries(${example} PRIVATE ${dependencies})
    target_compile_definitions(${example} PRIVATE "EXAMPLE_TITLE=\"${example}\"")
endfunction()


# point cloud
add_subdirectory(Tutorial_101_PointCloud)
add_subdirectory(Tutorial_102_PointCloud_Property)
add_subdirectory(Tutorial_103_PointCloud_IO)

# mesh
add_subdirectory(Tutorial_104_SurfaceMesh)
add_subdirectory(Tutorial_105_SurfaceMesh_Connectivity)
add_subdirectory(Tutorial_106_SurfaceMesh_Property)
add_subdirectory(Tutorial_107_SurfaceMesh_IO)

# graph
add_subdirectory(Tutorial_108_Graph)
add_subdirectory(Tutorial_109_Graph_Connectivity)
add_subdirectory(Tutorial_110_Graph_Property)
add_subdirectory(Tutorial_111_Graph_IO)

# polyhedral mesh
add_subdirectory(Tutorial_112_PolyMesh)
add_subdirectory(Tutorial_113_PolyMesh_Connectivity)
add_subdirectory(Tutorial_114_PolyMesh_Property)
add_subdirectory(Tutorial_115_PolyMesh_IO)

# ui, viewer, and camera
add_subdirectory(Tutorial_201_Viewer_default)
if(NOT Easy3D_ENABLE_VULKAN)   # needs ImGui and the callback_event_* hooks: docs/vulkan.md, W7/W10
    add_subdirectory(Tutorial_202_Viewer_imgui)
    add_subdirectory(Tutorial_203_Viewer_wxWidgets) # needs wxWidgets
endif()
if (Easy3D_ENABLE_QT)
    add_subdirectory(Tutorial_204_Viewer_Qt)
endif ()
#add_subdirectory(Tutorial_205_MultiView)   # to be implemented...
add_subdirectory(Tutorial_206_CameraInterpolation)
add_subdirectory(Tutorial_207_RealCamera)
add_subdirectory(Tutorial_208_Offscreen)

# drawable objects and basic rendering
add_subdirectory(Tutorial_301_Drawables)
add_subdirectory(Tutorial_302_Imposters)
add_subdirectory(Tutorial_303_ScalarField)
add_subdirectory(Tutorial_304_VectorField)
add_subdirectory(Tutorial_305_Texture)
add_subdirectory(Tutorial_306_ImageViewer)
add_subdirectory(Tutorial_307_CrossSection)
add_subdirectory(Tutorial_308_TexturedMesh)
add_subdirectory(Tutorial_309_TextRendering)
add_subdirectory(Tutorial_310_TextMesher)
add_subdirectory(Tutorial_311_Animation)
add_subdirectory(Tutorial_312_MultiThread)

# user interaction
add_subdirectory(Tutorial_401_ModelPicker)
add_subdirectory(Tutorial_402_FacePicker)
add_subdirectory(Tutorial_403_PointSelection)
add_subdirectory(Tutorial_404_VirtualScanner)
add_subdirectory(Tutorial_405_ObjectManipulation)
add_subdirectory(Tutorial_406_CollisionDetection)

# rendering techniques
if(NOT Easy3D_ENABLE_VULKAN)   # need draw() hooks, protected viewer state and the effects: docs/vulkan.md, W7/W10
    add_subdirectory(Tutorial_501_AmbientOcclusion)
    add_subdirectory(Tutorial_502_HardShadow)
    add_subdirectory(Tutorial_503_SoftShadow)
    add_subdirectory(Tutorial_504_Transparency)
    add_subdirectory(Tutorial_505_EyeDomeLighting)
endif()
add_subdirectory(Tutorial_506_DepthMap)

# algorithms - core
add_subdirectory(Tutorial_601_Tessellator)
add_subdirectory(Tutorial_602_ConvexPartition)
add_subdirectory(Tutorial_603_Curves)

# algorithms - of point cloud
add_subdirectory(Tutorial_701_Cloud_NormalEstimation)
add_subdirectory(Tutorial_702_Cloud_SurfaceReconstruction)
add_subdirectory(Tutorial_703_Cloud_PlaneExtraction)
#add_subdirectory(Tutorial_704_Cloud_DownSampling)    #TODO

# algorithms - surface mesh
#add_subdirectory(Tutorial_801_Mesh_Curvature)        #TODO
#add_subdirectory(Tutorial_802_Mesh_Smoothing)        #TODO
#add_subdirectory(Tutorial_803_Mesh_Fairing)          #TODO
#add_subdirectory(Tutorial_804_Mesh_HoleFilling)      #TODO
#add_subdirectory(Tutorial_805_Mesh_Remeshing)        #TODO
#add_subdirectory(Tutorial_806_Mesh_Simplification)   #TODO
#add_subdirectory(Tutorial_807_Mesh_Subdidivision)    #TODO


# Bounded smoke tests of the interactive tutorials (Vulkan backend). Each test is built from the tutorial's own
# sources with a generated main() (tests/native_tutorials) that stops after --frames N; tutorials stay unchanged.
if(Easy3D_ENABLE_VULKAN AND Easy3D_BUILD_VULKAN_TESTS)
    include("${Easy3D_ROOT}/tests/native_tutorials/test_sources.cmake")
    set(validation_args)
    if(Easy3D_VULKAN_TEST_VALIDATION)
        set(validation_args --validation)
    endif()
    foreach(tutorial Tutorial_201_Viewer_default Tutorial_206_CameraInterpolation Tutorial_207_RealCamera Tutorial_301_Drawables Tutorial_302_Imposters Tutorial_303_ScalarField Tutorial_304_VectorField Tutorial_305_Texture Tutorial_306_ImageViewer Tutorial_307_CrossSection Tutorial_308_TexturedMesh Tutorial_309_TextRendering Tutorial_310_TextMesher Tutorial_311_Animation Tutorial_312_MultiThread Tutorial_401_ModelPicker Tutorial_402_FacePicker Tutorial_403_PointSelection Tutorial_404_VirtualScanner Tutorial_405_ObjectManipulation Tutorial_406_CollisionDetection Tutorial_506_DepthMap Tutorial_601_Tessellator Tutorial_602_ConvexPartition Tutorial_603_Curves Tutorial_701_Cloud_NormalEstimation Tutorial_702_Cloud_SurfaceReconstruction Tutorial_703_Cloud_PlaneExtraction)
        string(REGEX MATCH "^Tutorial_([0-9]+)_" tutorial_prefix "${tutorial}")
        set(test_name vulkan.tutorial${CMAKE_MATCH_1})
        set(test_target ${tutorial}_test)
        set(directory "${Easy3D_ROOT}/tutorials/${tutorial}")
        native_tutorial_test_source(${tutorial} test_source)
        add_executable(${test_target} "${test_source}")
        target_include_directories(${test_target} PRIVATE "${directory}")
        if(tutorial STREQUAL "Tutorial_404_VirtualScanner")
            native_scanner_test_source(scanner_source)
            target_sources(${test_target} PRIVATE "${scanner_source}")
        elseif(EXISTS "${directory}/viewer.cpp")
            target_sources(${test_target} PRIVATE "${directory}/viewer.cpp")
        endif()
        if(EXISTS "${directory}/read_bundler_file.cpp")
            target_sources(${test_target} PRIVATE "${directory}/read_bundler_file.cpp")
        endif()
        target_compile_definitions(${test_target} PRIVATE EXAMPLE_TITLE="${tutorial}")
        target_link_libraries(${test_target} PRIVATE easy3d::viewer)
        set_target_properties(${test_target} PROPERTIES FOLDER "tests")
        add_test(NAME ${test_name} COMMAND ${test_target} --frames 3 ${validation_args})
        set_tests_properties(${test_name} PROPERTIES LABELS desktop TIMEOUT 120)
    endforeach()
endif()
