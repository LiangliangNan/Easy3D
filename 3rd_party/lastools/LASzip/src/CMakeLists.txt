###############################################################################
#
# src/CMakeLists.txt controls building of laszip library
#
# Copyright (c) 2009 Mateusz Loskot <mateusz@loskot.net>
#
###############################################################################
set(LASZIP_SOURCES
    arithmeticdecoder.cpp
    arithmeticencoder.cpp
    arithmeticmodel.cpp
    integercompressor.cpp
    lasindex.cpp
    lascopc.cpp
    lasinterval.cpp
    lasquadtree.cpp
    lasreaditemcompressed_v1.cpp
    lasreaditemcompressed_v2.cpp
    lasreaditemcompressed_v3.cpp
    lasreaditemcompressed_v4.cpp
    lasreadpoint.cpp
    laswriteitemcompressed_v1.cpp
    laswriteitemcompressed_v2.cpp
    laswriteitemcompressed_v3.cpp
    laswriteitemcompressed_v4.cpp
    laswritepoint.cpp
    laszip.cpp
    laszip_dll.cpp
    mydefs.cpp
)

add_definitions(-DLASZIPDLL_EXPORTS)
add_definitions(-DUNORDERED)

if(HAVE_UNORDERED_MAP)
    add_definitions(-DHAVE_UNORDERED_MAP=1)
endif(HAVE_UNORDERED_MAP)
LASZIP_ADD_LIBRARY(${LASZIP_BASE_LIB_NAME} ${LASZIP_SOURCES})
