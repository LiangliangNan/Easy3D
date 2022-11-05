get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(${module}_HEADERS
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

set(${module}_SOURCES
    CmdLineParser.cpp
    Factor.cpp
    Geometry.cpp
    MarchingCubes.cpp
    )

add_3rdparty_module(3rd_${module} "${${module}_SOURCES}" "${${module}_HEADERS}")


if (NOT ${Easy3D_BUILD_SHARED_LIBS})
	install(TARGETS 3rd_${module}
			# IMPORTANT: Add the library to the "export-set"
			EXPORT Easy3DTargets
			LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
			COMPONENT dev)
endif ()
