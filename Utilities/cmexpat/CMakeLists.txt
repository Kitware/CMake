project(CMEXPAT)

set(expat_SRCS
  xmlparse.c
  xmltok.c
  xmlrole.c
)

include(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(CMEXPAT_BIGENDIAN)

include_directories(
  "${CMEXPAT_BINARY_DIR}/.."
  )

if(WIN32)
  if(NOT BUILD_SHARED_LIBS)
    set(CM_EXPAT_STATIC 1)
  endif(NOT BUILD_SHARED_LIBS)
endif(WIN32)

configure_file(${CMEXPAT_SOURCE_DIR}/expatConfig.h.in
               ${CMEXPAT_BINARY_DIR}/expatConfig.h)
configure_file(${CMEXPAT_SOURCE_DIR}/expatDllConfig.h.in
               ${CMEXPAT_BINARY_DIR}/expatDllConfig.h)
configure_file(${CMEXPAT_SOURCE_DIR}/.NoDartCoverage
               ${CMEXPAT_BINARY_DIR}/.NoDartCoverage)
configure_file(${CMEXPAT_SOURCE_DIR}/cm_expat_mangle.h
               ${CMEXPAT_BINARY_DIR}/cm_expat_mangle.h)
configure_file(${CMEXPAT_SOURCE_DIR}/expat.h
               ${CMEXPAT_BINARY_DIR}/expat.h)

add_library(cmexpat ${expat_SRCS})
install(FILES COPYING DESTINATION ${CMake_DOC_DEST}/cmexpat)
