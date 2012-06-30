project(CMZLIB)

include_directories(
  "${CMZLIB_SOURCE_DIR}"
  "${CMZLIB_SOURCE_DIR}/.."
  "${CMZLIB_BINARY_DIR}/.."
  )

# source files for zlib
set(ZLIB_SRCS
  adler32.c  compress.c  crc32.c  deflate.c  gzio.c   inffast.c
  inflate.c  inftrees.c  trees.c  uncompr.c  zutil.c
  )

# for windows add the .def and .rc files to the source list
# if building shared libs
if(WIN32)
  if(BUILD_SHARED_LIBS)
    set(ZLIB_DLL 1)
    if(NOT UNIX)
      if(NOT BORLAND)
        if(NOT MINGW)
          set(ZLIB_SRCS ${ZLIB_SRCS} zlib.def zlib.rc  )
        endif(NOT MINGW)
      endif(NOT BORLAND)
    endif(NOT UNIX)
  endif(BUILD_SHARED_LIBS)
endif(WIN32)

configure_file(${CMZLIB_SOURCE_DIR}/.NoDartCoverage
  ${CMZLIB_BINARY_DIR}/.NoDartCoverage)
configure_file(${CMZLIB_SOURCE_DIR}/zlibDllConfig.h.in
  ${CMZLIB_BINARY_DIR}/zlibDllConfig.h)

foreach(name zlib zconf cm_zlib_mangle)
  configure_file(${CMZLIB_SOURCE_DIR}/${name}.h
    ${CMZLIB_BINARY_DIR}/${name}.h COPYONLY)
endforeach(name)


add_library(cmzlib ${ZLIB_SRCS})

install(FILES Copyright.txt DESTINATION ${CMake_DOC_DEST}/cmzlib)
