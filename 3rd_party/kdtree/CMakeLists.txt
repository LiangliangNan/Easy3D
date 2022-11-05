get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(${module}_HEADERS
    ANN/ANN.h
    ANN/ANNperf.h
    ANN/ANNx.h
    ANN/bd_tree.h
    ANN/kd_fix_rad_search.h
    ANN/kd_pr_search.h
    ANN/kd_search.h
    ANN/kd_split.h
    ANN/kd_tree.h
    ANN/kd_util.h
    ANN/pr_queue.h
    ANN/pr_queue_k.h
    ETH_Kd_Tree/kdTree.h
    ETH_Kd_Tree/PriorityQueue.h
    ETH_Kd_Tree/QueryGrid.h
    ETH_Kd_Tree/vector2D.h
    ETH_Kd_Tree/vector3D.h
    FLANN/config.h
    FLANN/defines.h
    FLANN/flann.hpp
    FLANN/general.h
    FLANN/algorithms/all_indices.h
    FLANN/algorithms/autotuned_index.h
    FLANN/algorithms/center_chooser.h
    FLANN/algorithms/composite_index.h
    FLANN/algorithms/dist.h
    FLANN/algorithms/hierarchical_clustering_index.h
    FLANN/algorithms/kdtree_index.h
    FLANN/algorithms/kdtree_single_index.h
    FLANN/algorithms/kmeans_index.h
    FLANN/algorithms/linear_index.h
    FLANN/algorithms/lsh_index.h
    FLANN/algorithms/nn_index.h
    FLANN/nn/ground_truth.h
    FLANN/nn/index_testing.h
    FLANN/util/allocator.h
    FLANN/util/any.h
    FLANN/util/dynamic_bitset.h
    FLANN/util/heap.h
    FLANN/util/logger.h
    FLANN/util/lsh_table.h
    FLANN/util/matrix.h
    FLANN/util/params.h
    FLANN/util/random.h
    FLANN/util/result_set.h
    FLANN/util/sampling.h
    FLANN/util/saving.h
    FLANN/util/serialization.h
    FLANN/util/timer.h
    nanoflann/nanoflann.hpp
    )

set(${module}_SOURCES
    ANN/ANN.cpp
    ANN/bd_fix_rad_search.cpp
    ANN/bd_pr_search.cpp
    ANN/bd_search.cpp
    ANN/bd_tree.cpp
    ANN/brute.cpp
    ANN/kd_dump.cpp
    ANN/kd_fix_rad_search.cpp
    ANN/kd_pr_search.cpp
    ANN/kd_search.cpp
    ANN/kd_split.cpp
    ANN/kd_tree.cpp
    ANN/kd_util.cpp
    ANN/perf.cpp
    ETH_Kd_Tree/kdTree.cpp
    )

add_3rdparty_module(3rd_${module} "${${module}_SOURCES}" "${${module}_HEADERS}")


if (NOT ${Easy3D_BUILD_SHARED_LIBS})
	install(TARGETS 3rd_${module}
			# IMPORTANT: Add the library to the "export-set"
			EXPORT Easy3DTargets
			LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
			COMPONENT dev)
endif ()
