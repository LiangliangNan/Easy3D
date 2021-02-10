cmake_minimum_required(VERSION 3.1)

project(3rd_poisson)


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


add_library(3rd_poisson STATIC ${poisson_SOURCES} ${poisson_HEADERS})

set_target_properties(3rd_poisson PROPERTIES FOLDER "3rd_party")

target_include_directories(3rd_poisson PRIVATE ${EASY3D_poisson_INCLUDE_DIR})


# may use OpenMP
include( ../../cmake/UseOpenMP.cmake )


if (MSVC)
    target_compile_definitions(3rd_poisson PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
        )
endif()