get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(${module}_HEADERS
    LASlib/inc/lasdefinitions.hpp
    LASlib/inc/lasfilter.hpp
    LASlib/inc/lasreader_asc.hpp
    LASlib/inc/lasreader_bil.hpp
    LASlib/inc/lasreader_bin.hpp
    LASlib/inc/lasreader_dtm.hpp
    LASlib/inc/lasreader_las.hpp
    LASlib/inc/lasreader_ply.hpp
    LASlib/inc/lasreader_qfit.hpp
    LASlib/inc/lasreader_shp.hpp
    LASlib/inc/lasreader_txt.hpp
    LASlib/inc/lasreader.hpp
    LASlib/inc/lasreaderbuffered.hpp
    LASlib/inc/lasreadermerged.hpp
    LASlib/inc/lasreaderpipeon.hpp
    LASlib/inc/lasreaderstored.hpp
    LASlib/inc/lastransform.hpp
    LASlib/inc/lasutility.hpp
    LASlib/inc/laswaveform13reader.hpp
    LASlib/inc/laswaveform13writer.hpp
    LASlib/inc/laswriter_bin.hpp
    LASlib/inc/laswriter_las.hpp
    LASlib/inc/laswriter_qfit.hpp
    LASlib/inc/laswriter_txt.hpp
    LASlib/inc/laswriter_wrl.hpp
    LASlib/inc/laswriter.hpp
    LASlib/inc/laswritercompatible.hpp
    LASzip/src/arithmeticdecoder.hpp
    LASzip/src/arithmeticencoder.hpp
    LASzip/src/arithmeticmodel.hpp
    LASzip/src/bytestreamin_array.hpp
    LASzip/src/bytestreamin_file.hpp
    LASzip/src/bytestreamin_istream.hpp
    LASzip/src/bytestreamin.hpp
    LASzip/src/bytestreaminout_file.hpp
    LASzip/src/bytestreaminout.hpp
    LASzip/src/bytestreamout_array.hpp
    LASzip/src/bytestreamout_file.hpp
    LASzip/src/bytestreamout_nil.hpp
    LASzip/src/bytestreamout_ostream.hpp
    LASzip/src/bytestreamout.hpp
    LASzip/src/integercompressor.hpp
    LASzip/src/lasattributer.hpp
    LASzip/src/lasindex.hpp
    LASzip/src/lasinterval.hpp
    LASzip/src/laspoint.hpp
    LASzip/src/lasquadtree.hpp
    LASzip/src/lasquantizer.hpp
    LASzip/src/lasreaditem.hpp
    LASzip/src/lasreaditemcompressed_v1.hpp
    LASzip/src/lasreaditemcompressed_v2.hpp
    LASzip/src/lasreaditemcompressed_v3.hpp
    LASzip/src/lasreaditemcompressed_v4.hpp
    LASzip/src/lasreaditemraw.hpp
    LASzip/src/lasreadpoint.hpp
    LASzip/src/laswriteitem.hpp
    LASzip/src/laswriteitemcompressed_v1.hpp
    LASzip/src/laswriteitemcompressed_v2.hpp
    LASzip/src/laswriteitemcompressed_v3.hpp
    LASzip/src/laswriteitemcompressed_v4.hpp
    LASzip/src/laswriteitemraw.hpp
    LASzip/src/laswritepoint.hpp
    LASzip/src/laszip_common_v1.hpp
    LASzip/src/laszip_common_v2.hpp
    LASzip/src/laszip_common_v3.hpp
    LASzip/src/laszip_decompress_selective_v3.hpp
    LASzip/src/laszip.hpp
    LASzip/src/mydefs.hpp
    )

set(${module}_SOURCES
    LASlib/src/fopen_compressed.cpp
    LASlib/src/lasfilter.cpp
    LASlib/src/lasreader_asc.cpp
    LASlib/src/lasreader_bil.cpp
    LASlib/src/lasreader_bin.cpp
    LASlib/src/lasreader_dtm.cpp
    LASlib/src/lasreader_las.cpp
    LASlib/src/lasreader_ply.cpp
    LASlib/src/lasreader_qfit.cpp
    LASlib/src/lasreader_shp.cpp
    LASlib/src/lasreader_txt.cpp
    LASlib/src/lasreader.cpp
    LASlib/src/lasreaderbuffered.cpp
    LASlib/src/lasreadermerged.cpp
    LASlib/src/lasreaderpipeon.cpp
    LASlib/src/lasreaderstored.cpp
    LASlib/src/lastransform.cpp
    LASlib/src/lasutility.cpp
    LASlib/src/laswaveform13reader.cpp
    LASlib/src/laswaveform13writer.cpp
    LASlib/src/laswriter_bin.cpp
    LASlib/src/laswriter_las.cpp
    LASlib/src/laswriter_qfit.cpp
    LASlib/src/laswriter_txt.cpp
    LASlib/src/laswriter_wrl.cpp
    LASlib/src/laswriter.cpp
    LASlib/src/laswritercompatible.cpp
    LASzip/src/arithmeticdecoder.cpp
    LASzip/src/arithmeticencoder.cpp
    LASzip/src/arithmeticmodel.cpp
    LASzip/src/integercompressor.cpp
    LASzip/src/lasindex.cpp
    LASzip/src/lasinterval.cpp
    LASzip/src/lasquadtree.cpp
    LASzip/src/lasreaditemcompressed_v1.cpp
    LASzip/src/lasreaditemcompressed_v2.cpp
    LASzip/src/lasreaditemcompressed_v3.cpp
    LASzip/src/lasreaditemcompressed_v4.cpp
    LASzip/src/lasreadpoint.cpp
    LASzip/src/laswriteitemcompressed_v1.cpp
    LASzip/src/laswriteitemcompressed_v2.cpp
    LASzip/src/laswriteitemcompressed_v3.cpp
    LASzip/src/laswriteitemcompressed_v4.cpp
    LASzip/src/laswritepoint.cpp
    LASzip/src/laszip.cpp
    )

add_3rdparty_module(3rd_${module} "${${module}_SOURCES}" "${${module}_HEADERS}")
target_include_directories(3rd_${module} PRIVATE 
		${CMAKE_CURRENT_LIST_DIR}/LASzip/src 
		${CMAKE_CURRENT_LIST_DIR}/LASlib/inc
		)


if (NOT ${Easy3D_BUILD_SHARED_LIBS})
	install(TARGETS 3rd_${module}
			# IMPORTANT: Add the library to the "export-set"
			EXPORT Easy3DTargets
			LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
			COMPONENT dev)
endif ()
