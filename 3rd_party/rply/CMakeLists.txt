get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(${module}_HEADERS
		rply.h
		)

set(${module}_SOURCES
		rply.c
		)

add_3rdparty_module(3rd_${module} "${${module}_SOURCES}" "${${module}_HEADERS}")


if (NOT ${Easy3D_BUILD_SHARED_LIBS})
	install(TARGETS 3rd_rply
			# IMPORTANT: Add the library to the "export-set"
			EXPORT Easy3DTargets
			LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
			COMPONENT dev)
endif ()
