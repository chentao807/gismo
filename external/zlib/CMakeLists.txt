######################################################################
## CMakeLists.txt ---
## This file is part of the G+Smo library. 
## 
## Author: Angelos Mantzaflaris 
## Copyright (C) 2012 - 2015 RICAM-Linz.
######################################################################

project(zlib)

## #################################################################
## Collect files
## #################################################################

set(ZLIB_SOURCES
${gismo_externals}/zlib/adler32.c
${gismo_externals}/zlib/compress.c
${gismo_externals}/zlib/crc32.c
${gismo_externals}/zlib/deflate.c
${gismo_externals}/zlib/gzclose.c
${gismo_externals}/zlib/gzlib.c
${gismo_externals}/zlib/gzread.c
${gismo_externals}/zlib/gzstream.cpp
${gismo_externals}/zlib/gzwrite.c
${gismo_externals}/zlib/infback.c
${gismo_externals}/zlib/inffast.c
${gismo_externals}/zlib/inflate.c
${gismo_externals}/zlib/inftrees.c
${gismo_externals}/zlib/trees.c
${gismo_externals}/zlib/uncompr.c
${gismo_externals}/zlib/zutil.c
)

add_library(${PROJECT_NAME} OBJECT ${ZLIB_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "Z_PREFIX")
IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
  SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS -fPIC)
ENDIF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )

if(GISMO_WITH_ONURBS)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "MY_ZCALLOC")
endif()

#if(GISMO_BUILD_LIB)

#set (gismo_SOURCES ${gismo_SOURCES} ${ZLIB_SOURCES}
#     CACHE INTERNAL "gismo source files")

#endif()

set(gismo_EXTENSIONS ${gismo_EXTENSIONS} $<TARGET_OBJECTS:zlib>
    CACHE INTERNAL "Gismo extensions to be included")

