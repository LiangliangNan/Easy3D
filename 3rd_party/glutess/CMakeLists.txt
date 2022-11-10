get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(${module}_HEADERS
		glutess.h
		)

set(${module}_SOURCES
        source/dict-list.h
        source/dict.c
        source/dict.h
        source/geom.c
        source/geom.h
        source/memalloc.c
        source/memalloc.h
        source/mesh.c
        source/mesh.h
        source/normal.c
        source/normal.h
        #source/priorityq-heap.c
        #source/priorityq-heap.h
        source/priorityq-sort.h
        source/priorityq.c
        source/priorityq.h
        source/render.c
        source/render.h
        source/sweep.c
        source/sweep.h
        source/tess.c
        source/tess.h
        source/tessmono.c
        source/tessmono.h
        )

add_3rdparty_module(3rd_${module} "${${module}_SOURCES}" "${${module}_HEADERS}")
target_include_directories(3rd_${module} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if (NOT ${Easy3D_BUILD_SHARED_LIBS})
	install(TARGETS 3rd_${module}
			# IMPORTANT: Add the library to the "export-set"
			EXPORT Easy3DTargets
			LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
			COMPONENT dev)
endif ()
