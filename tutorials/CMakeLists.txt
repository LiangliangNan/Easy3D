cmake_minimum_required(VERSION 3.1)

# point cloud
add_subdirectory("Tutorial_101_PointCloud")
add_subdirectory("Tutorial_102_PointCloud_Property")
add_subdirectory("Tutorial_103_PointCloud_IO")

# mesh
add_subdirectory("Tutorial_201_Mesh")
add_subdirectory("Tutorial_202_Mesh_Connectivity")
add_subdirectory("Tutorial_203_Mesh_Property")
add_subdirectory("Tutorial_204_Mesh_IO")

# viewer & drawing
add_subdirectory("Tutorial_301_Viewer")
add_subdirectory("Tutorial_302_Viewer_imgui")
add_subdirectory("Tutorial_303_Viewer_Qt")
add_subdirectory("Tutorial_304_RealCamera")
add_subdirectory("Tutorial_305_DepthImage")
add_subdirectory("Tutorial_306_ImageViewer")

# basic rendering
add_subdirectory("Tutorial_401_Drawable")
add_subdirectory("Tutorial_402_ScalarField")
add_subdirectory("Tutorial_403_VectorField")
add_subdirectory("Tutorial_404_Imposters")
add_subdirectory("Tutorial_405_Texture")

# rendering techniques
add_subdirectory("Tutorial_501_EyeDomeLighting")
add_subdirectory("Tutorial_502_AmbientOcclusion")
add_subdirectory("Tutorial_503_HardShadow")
add_subdirectory("Tutorial_504_SoftShadow")
add_subdirectory("Tutorial_505_Transparency")

# point cloud algorithms
add_subdirectory("Tutorial_601_PointCloud_NormalEstimation")
add_subdirectory("Tutorial_602_PointCloud_SurfaceReconstruction")
add_subdirectory("Tutorial_603_PointCloud_PlaneExtraction")
