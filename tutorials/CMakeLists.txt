cmake_minimum_required(VERSION 3.1)

# point cloud
add_subdirectory("Tutorial_101_PointCloud")
add_subdirectory("Tutorial_102_PointCloud_Properties")
add_subdirectory("Tutorial_103_PointCloud_FileIO")

# mesh
add_subdirectory("Tutorial_201_Mesh")
add_subdirectory("Tutorial_202_Mesh_Connectivities")
add_subdirectory("Tutorial_203_Mesh_Properties")
add_subdirectory("Tutorial_204_Mesh_FileIO")

# viewer & drawing
add_subdirectory("Tutorial_301_Viewer")
add_subdirectory("Tutorial_302_Drawables")
add_subdirectory("Tutorial_303_ScalarFields")
add_subdirectory("Tutorial_304_VectorFields")
add_subdirectory("Tutorial_305_RealCamera")

# rendering techniques
add_subdirectory("Tutorial_401_EyeDomeLighting")
add_subdirectory("Tutorial_402_AmbientOcclusion")
add_subdirectory("Tutorial_403_HardShadow")
add_subdirectory("Tutorial_404_SoftShadow")
add_subdirectory("Tutorial_405_Transparency")

