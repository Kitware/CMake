INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

IF ("${PROJECT_SOURCE_DIR}" STREQUAL "${ANOTHER_PROJ_SOURCE_DIR}")
  SET(BUILD_SHARED_LIBS 1)

  # Construct a source file outside the tree whose full path is close to
  # the path length limit.  This will cause the full path to the object
  # file in the build tree to exceed the maximum path length which will
  # test cmLocalGenerator::CreateSafeUniqueObjectFileName.
  GET_FILENAME_COMPONENT(DEEPDIR
    ${OutOfSource_BINARY_DIR}/../OutOfSourceDeep/deeper ABSOLUTE)
  # MAXPATH = 250 less 25 for /and/deeper/simple.cxx part and small safety
  MATH(EXPR MAXPATH "250 - 25")
  STRING(LENGTH "${DEEPDIR}" DEEPDIR_LEN)
  WHILE("${DEEPDIR_LEN}" LESS "${MAXPATH}")
    SET(DEEPDIR ${DEEPDIR}/and/deeper)
    STRING(LENGTH "${DEEPDIR}" DEEPDIR_LEN)
  ENDWHILE("${DEEPDIR_LEN}" LESS "${MAXPATH}")
  SET(DEEPSRC ${DEEPDIR}/simple.cxx)
  STRING(LENGTH "${DEEPSRC}" DEEPSRC_LEN)
  CONFIGURE_FILE(simple.cxx.in ${DEEPSRC} COPYONLY)

  ADD_LIBRARY(testlib testlib.cxx)
  ADD_EXECUTABLE (simple simple.cxx ../simple.cxx ${DEEPSRC})
  TARGET_LINK_LIBRARIES(simple testlib outlib)
ENDIF ("${PROJECT_SOURCE_DIR}" STREQUAL "${ANOTHER_PROJ_SOURCE_DIR}")

# test getting a definition from a subdir
SET (WEASELS SIZZLING)
