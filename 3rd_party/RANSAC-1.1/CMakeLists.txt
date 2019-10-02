cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(ransac_HEADERS
           BasePrimitiveShape.h
           basic.h
           Bitmap.h
           BitmapPrimitiveShape.h
           Candidate.h
           Cone.h
           ConePrimitiveShape.h
           ConePrimitiveShapeConstructor.h
           Cylinder.h
           CylinderPrimitiveShape.h
           CylinderPrimitiveShapeConstructor.h
           FlatNormalThreshPointCompatibilityFunc.h
           Grid.h
           IndexIterator.h
           LevMarFitting.h
           LevMarFunc.h
           LevMarLSWeight.h
           LowStretchSphereParametrization.h
           LowStretchTorusParametrization.h
           Octree.h
           Plane.h
           PlanePrimitiveShape.h
           PlanePrimitiveShapeConstructor.h
           PointCloud.h
           PrimitiveShape.h
           PrimitiveShapeConstructor.h
           PrimitiveShapeVisitor.h
           RansacShapeDetector.h
           RebuildAACubeTreeStrategy.h
           ScoreAACubeTreeStrategy.h
           ScoreComputer.h
           ScorePrimitiveShapeVisitor.h
           SimpleTorusParametrization.h
           Sphere.h
           SpherePrimitiveShape.h
           SpherePrimitiveShapeConstructor.h
           Torus.h
           TorusPrimitiveShape.h
           TorusPrimitiveShapeConstructor.h
           GfxTL/AABox.h
           GfxTL/AABox.hpp
           GfxTL/AACube.h
           GfxTL/AACube.hpp
           GfxTL/AACubeTree.h
           GfxTL/AACubeTree.hpp
           GfxTL/AAKdTree.h
           GfxTL/AAKdTree.hpp
           GfxTL/AAPlane.h
           GfxTL/AAPlane.hpp
           GfxTL/Array.h
           GfxTL/ArrayArithmetic.h
           GfxTL/BaseTree.h
           GfxTL/BaseTree.hpp
           GfxTL/BBoxBuildInformationTreeStrategy.h
           GfxTL/BBoxDistanceKdTreeStrategy.h
           GfxTL/BucketSizeMaxLevelSubdivisionTreeStrategy.h
           GfxTL/BucketSizeSubdivisionTreeStrategy.h
           GfxTL/CellBBoxBuildInformationKdTreeStrategy.h
           GfxTL/CellCenterAACubeTreeStrategy.h
           GfxTL/CellLevelTreeStrategy.h
           GfxTL/CellParentTreeStrategy.h
           GfxTL/CellRangeDataTreeStrategy.h
           GfxTL/CellSizeDataTreeStrategy.h
           GfxTL/Covariance.h
           GfxTL/DynVectorKernel.h
           GfxTL/FaceNeighborAACubeTreeStrategy.h
           GfxTL/FlatCopyVector.h
           GfxTL/Frame.h
           GfxTL/HyperplaneCoordinateSystem.h
           GfxTL/ImmediateTreeDataKernels.h
           GfxTL/IncrementalDistanceKdTreeStrategy.h
           GfxTL/IndexedIterator.h
           GfxTL/IndexedTreeDataKernels.h
           GfxTL/IndexedTreeDataStrategy.h
           GfxTL/Jacobi.h
           GfxTL/KdTree.h
           GfxTL/KdTree.hpp
           GfxTL/L1Norm.h
           GfxTL/L2Norm.h
           GfxTL/LevelBuildInformationTreeStrategy.h
           GfxTL/LimitedHeap.h
           GfxTL/MathHelper.h
           GfxTL/MatrixXX.h
           GfxTL/MaxIntervalSplittingKdTreeStrategy.h
           GfxTL/MaxLevelSubdivisionTreeStrategy.h
           GfxTL/MaxNorm.h
           GfxTL/Mean.h
           GfxTL/NearestNeighbor.h
           GfxTL/NearestNeighbors.h
           GfxTL/NullClass.h
           GfxTL/NullStrategy.h
           GfxTL/NullStrategy.hpp
           GfxTL/NullTreeStrategy.h
           GfxTL/OnOff.h
           GfxTL/Orientation.h
           GfxTL/Plane.h
           GfxTL/Plane.hpp
           GfxTL/ScalarTypeConversion.h
           GfxTL/ScalarTypeDeferer.h
           GfxTL/ShuffleIndicesTreeDataKernelStrategy.h
           GfxTL/SlidingMidpointSplittingKdTreeStrategy.h
           GfxTL/StdContainerAdaptor.h
           GfxTL/StdOverrides.h
           GfxTL/Swap.h
           GfxTL/VectorKernel.h
           GfxTL/VectorXD.h
           GfxTL/WeightFunc.h
           MiscLib/AlignedAllocator.h
           MiscLib/NoShrinkVector.h
           MiscLib/Pair.h
           MiscLib/Performance.h
           MiscLib/Random.h
           MiscLib/RefCount.h
           MiscLib/RefCounted.h
           MiscLib/RefCountPtr.h
           MiscLib/Vector.h
           MiscLib/AlignedAllocator.h
           GfxTL/VectorXD.h
           GfxTL/MatrixXX.h
           GfxTL/StdOverrides.h
           GfxTL/MathHelper.h
           GfxTL/NullClass.h
           MiscLib/Performance.h
           MiscLib/RefCount.h
           MiscLib/NoShrinkVector.h
           MiscLib/RefCountPtr.h
           MiscLib/Random.h
           GfxTL/AABox.h
           GfxTL/AACube.h
           GfxTL/IndexedIterator.h
           MiscLib/Pair.h
           MiscLib/RefCounted.h
           GfxTL/AACubeTree.h
           GfxTL/BaseTree.h
           GfxTL/VectorKernel.h
           GfxTL/ScalarTypeConversion.h
           GfxTL/ScalarTypeDeferer.h
           GfxTL/Array.h
           GfxTL/LevelBuildInformationTreeStrategy.h
           GfxTL/CellSizeDataTreeStrategy.h
           GfxTL/CellRangeDataTreeStrategy.h
           GfxTL/ImmediateTreeDataKernels.h
           GfxTL/Swap.h
           GfxTL/IndexedTreeDataKernels.h
           GfxTL/BucketSizeMaxLevelSubdivisionTreeStrategy.h
           GfxTL/CellLevelTreeStrategy.h
           GfxTL/CellCenterAACubeTreeStrategy.h
           GfxTL/NullTreeStrategy.h
           GfxTL/ShuffleIndicesTreeDataKernelStrategy.h
           GfxTL/HyperplaneCoordinateSystem.h
           GfxTL/Plane.h
           GfxTL/Covariance.h
           GfxTL/WeightFunc.h
           GfxTL/Jacobi.h
           GfxTL/Mean.h
           GfxTL/Frame.h
           GfxTL/KdTree.h
           GfxTL/LimitedHeap.h
           GfxTL/FlatCopyVector.h
           GfxTL/StdContainerAdaptor.h
           GfxTL/NearestNeighbor.h
           GfxTL/BBoxDistanceKdTreeStrategy.h
           GfxTL/IncrementalDistanceKdTreeStrategy.h
           GfxTL/MaxIntervalSplittingKdTreeStrategy.h
           GfxTL/CellBBoxBuildInformationKdTreeStrategy.h
           GfxTL/BBoxBuildInformationTreeStrategy.h
           GfxTL/L2Norm.h
           GfxTL/BucketSizeSubdivisionTreeStrategy.h
           GfxTL/NearestNeighbors.h  
  )

set(ransac_SOURCES
           Bitmap.cpp
           BitmapPrimitiveShape.cpp
           Candidate.cpp
           Cone.cpp
           ConePrimitiveShape.cpp
           ConePrimitiveShapeConstructor.cpp
           Cylinder.cpp
           CylinderPrimitiveShape.cpp
           CylinderPrimitiveShapeConstructor.cpp
           LowStretchSphereParametrization.cpp
           LowStretchTorusParametrization.cpp
           Plane.cpp
           PlanePrimitiveShape.cpp
           PlanePrimitiveShapeConstructor.cpp
           PointCloud.cpp
           RansacShapeDetector.cpp
           SimpleTorusParametrization.cpp
           solve.cpp
           Sphere.cpp
           SpherePrimitiveShape.cpp
           SpherePrimitiveShapeConstructor.cpp
           Torus.cpp
           TorusPrimitiveShape.cpp
           TorusPrimitiveShapeConstructor.cpp
           MiscLib/Random.cpp
           MiscLib/RefCount.cpp
    )


add_library(${PROJECT_NAME} STATIC ${ransac_SOURCES} ${ransac_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "3rd_party")

set(RANSAC_INCLUDE_DIR   ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${RANSAC_INCLUDE_DIR})


# Liangliang: Otherwise there will be many related errors
if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++11-narrowing")
endif()


# ------------------------------------------------------------------------------
# OpenMP
# ------------------------------------------------------------------------------
# Liangliang: In the original RANSAC implementation, OpenMP is enabled if
#             the DOPARALLEL micro is defined. However, it seems enabling
#             OpenMP causes the process loops infinitely on Windows (tested
#             with Microft Visual Studio 2017). So I just don't enable it.
find_package(OpenMP QUIET)
if (OPENMP_FOUND AND NOT WIN32)
    message("OpenMP found")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    # define DOPARALLEL to enable OpenMP
    target_compile_definitions(${PROJECT_NAME} PRIVATE DOPARALLEL)
endif()


if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
        )
endif()
