cmake_minimum_required(VERSION 3.1)

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
add_subdirectory(Tutorial_201_Viewer_imgui)
if (EASY3D_ENABLE_QT)
    add_subdirectory(Tutorial_202_Viewer_Qt)
endif ()
add_subdirectory(Tutorial_203_CompositeView)
add_subdirectory(Tutorial_204_CameraInterpolation)
add_subdirectory(Tutorial_205_RealCamera)

# drawable objects and basic rendering
add_subdirectory(Tutorial_301_Drawables)
add_subdirectory(Tutorial_302_Imposters)
add_subdirectory(Tutorial_303_ScalarField)
add_subdirectory(Tutorial_304_VectorField)
add_subdirectory(Tutorial_305_Texture)
add_subdirectory(Tutorial_306_Image)
add_subdirectory(Tutorial_307_Tessellator)
add_subdirectory(Tutorial_308_TexturedMesh)
add_subdirectory(Tutorial_309_TextRendering)
add_subdirectory(Tutorial_310_TextMesher)
add_subdirectory(Tutorial_311_Animation)

# user interaction
add_subdirectory(Tutorial_401_ModelPicker)
add_subdirectory(Tutorial_402_FacePicker)
add_subdirectory(Tutorial_403_PointSelection)

# rendering techniques
add_subdirectory(Tutorial_501_AmbientOcclusion)
add_subdirectory(Tutorial_502_HardShadow)
add_subdirectory(Tutorial_503_SoftShadow)
add_subdirectory(Tutorial_504_Transparency)
add_subdirectory(Tutorial_505_EyeDomeLighting)
add_subdirectory(Tutorial_506_DepthMaps)

# algorithms - of point cloud
add_subdirectory(Tutorial_601_Cloud_NormalEstimation)
add_subdirectory(Tutorial_602_Cloud_SurfaceReconstruction)
add_subdirectory(Tutorial_603_Cloud_PlaneExtraction)
#add_subdirectory(Tutorial_604_Cloud_DownSampling)    #TODO

# algorithms - surface mesh
#add_subdirectory(Tutorial_701_Mesh_Curvature)        #TODO
#add_subdirectory(Tutorial_702_Mesh_Smoothing)        #TODO
#add_subdirectory(Tutorial_703_Mesh_Fairing)          #TODO
#add_subdirectory(Tutorial_704_Mesh_HoleFilling)      #TODO
#add_subdirectory(Tutorial_705_Mesh_Remeshing)        #TODO
#add_subdirectory(Tutorial_706_Mesh_Simplification)   #TODO
#add_subdirectory(Tutorial_707_Mesh_Subdidivision)    #TODO