PROJECT(CMZLIB)

INCLUDE_DIRECTORIES(
  "${CMZLIB_SOURCE_DIR}"
  "${CMZLIB_SOURCE_DIR}/.."
  "${CMZLIB_BINARY_DIR}/.."
  )

# Tell the header files included from the .c files that they are being
# built.  Used to disable warnings since we do not want to change the
# code much.
ADD_DEFINITIONS(-DCMZLIB_IN_C)

# Match all headers for dependencies but complain about none.
INCLUDE_REGULAR_EXPRESSION("^.*$")

# Disable warnings on Borland to avoid changing 3rd party code.
IF(BORLAND)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w-")
ENDIF(BORLAND)

# source files for zlib
SET(ZLIB_SRCS
  adler32.c   gzio.c      inftrees.c  uncompr.c
  compress.c  infblock.c  infutil.c   zutil.c
  crc32.c     infcodes.c 
  deflate.c   inffast.c  
  inflate.c   trees.c
  )

# for windows add the .def and .rc files to the source list
IF(WIN32)
  IF(BUILD_SHARED_LIBS)
    SET(ZLIB_DLL 1)
    IF(NOT UNIX)
      IF(NOT BORLAND)
        IF(NOT MINGW)
          SET(ZLIB_SRCS ${ZLIB_SRCS} zlib.def zlib.rc  )
        ENDIF(NOT MINGW)
      ENDIF(NOT BORLAND) 
    ENDIF(NOT UNIX)   
  ENDIF(BUILD_SHARED_LIBS)
ENDIF(WIN32)

CONFIGURE_FILE(${CMZLIB_SOURCE_DIR}/.NoDartCoverage
  ${CMZLIB_BINARY_DIR}/.NoDartCoverage)
CONFIGURE_FILE(${CMZLIB_SOURCE_DIR}/zlibDllConfig.h.in
  ${CMZLIB_BINARY_DIR}/zlibDllConfig.h)

FOREACH(name zlib zconf cm_zlib_mangle)
  CONFIGURE_FILE(${CMZLIB_SOURCE_DIR}/${name}.h
    ${CMZLIB_BINARY_DIR}/${name}.h COPYONLY)
ENDFOREACH(name)


ADD_LIBRARY(cmzlib ${ZLIB_SRCS})
