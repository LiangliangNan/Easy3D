get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(${module}_HEADERS
		triangle.h
		)

set(${module}_SOURCES
		triangle.c
		)

add_3rdparty_module(3rd_${module} "${${module}_SOURCES}" "${${module}_HEADERS}")
target_compile_definitions(3rd_${module} PRIVATE TRILIBRARY)


if (NOT ${Easy3D_BUILD_SHARED_LIBS})
	install(TARGETS 3rd_triangle
			# IMPORTANT: Add the library to the "export-set"
			EXPORT Easy3DTargets
			LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
			COMPONENT dev)
endif ()
