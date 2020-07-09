cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(poisson_HEADERS
    Allocator.h
    Array.h
    BinaryNode.h
    BSplineData.h
    CmdLineParser.h
    Factor.h
    FunctionData.h
    Geometry.h
    MarchingCubes.h
    MAT.h
    MemoryUsage.h
    MultiGridOctreeData.h
    MyTime.h
    Octree.h
    Ply.h
    PointStream.h
    Polynomial.h
    PPolynomial.h
    SparseMatrix.h
    SurfaceTrimmer.h
    CmdLineParser.inl
    Geometry.inl
    Array.inl
    BSplineData.inl
    CmdLineParser.inl
    FunctionData.inl
    Geometry.inl
    MAT.inl
    MultiGridOctreeData.Evaluation.inl
    MultiGridOctreeData.inl
    MultiGridOctreeData.IsoSurface.inl
    MultiGridOctreeData.SortedTreeNodes.inl
    MultiGridOctreeData.System.inl
    MultiGridOctreeData.WeightedSamples.inl
    MyTreeInit.inl
    Octree.inl
    PointStream.inl
    Polynomial.inl
    PPolynomial.inl
    SparseMatrix.inl
    )

set(poisson_SOURCES
    CmdLineParser.cpp
    Factor.cpp
    Geometry.cpp
    MarchingCubes.cpp
#    PlyFile.cpp
    )


add_library(${PROJECT_NAME} STATIC ${poisson_SOURCES} ${poisson_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "3rd_party")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_poisson_INCLUDE_DIR})


# may use OpenMP
include( ../../cmake/UseOpenMP.cmake )


if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
        )
endif()
