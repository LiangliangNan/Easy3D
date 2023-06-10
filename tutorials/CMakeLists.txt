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
add_subdirectory(Tutorial_202_Viewer_imgui)
add_subdirectory(Tutorial_203_Viewer_wxWidgets)
if (Easy3D_ENABLE_QT)
    add_subdirectory(Tutorial_204_Viewer_Qt)
endif ()
add_subdirectory(Tutorial_205_MultiView)
add_subdirectory(Tutorial_206_CameraInterpolation)
add_subdirectory(Tutorial_207_RealCamera)

# drawable objects and basic rendering
add_subdirectory(Tutorial_301_Drawables)
add_subdirectory(Tutorial_302_Imposters)
add_subdirectory(Tutorial_303_ScalarField)
add_subdirectory(Tutorial_304_VectorField)
add_subdirectory(Tutorial_305_Texture)
add_subdirectory(Tutorial_306_Image)
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
add_subdirectory(Tutorial_501_AmbientOcclusion)
add_subdirectory(Tutorial_502_HardShadow)
add_subdirectory(Tutorial_503_SoftShadow)
add_subdirectory(Tutorial_504_Transparency)
add_subdirectory(Tutorial_505_EyeDomeLighting)
add_subdirectory(Tutorial_506_DepthMaps)

# algorithms - core
add_subdirectory(Tutorial_601_Tessellator)
add_subdirectory(Tutorial_602_ConvexPartition)

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