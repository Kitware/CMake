INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

IF ("${PROJECT_SOURCE_DIR}" STREQUAL "${ANOTHER_PROJ_SOURCE_DIR}")
  SET(BUILD_SHARED_LIBS 1)

  # Construct a source file outside the tree whose full path is close to
  # the path length limit.  This will cause the full path to the object
  # file in the build tree to exceed the maximum path length which will
  # test cmLocalGenerator::CreateSafeUniqueObjectFileName.
  GET_FILENAME_COMPONENT(DEEPDIR
    ${OutOfSource_BINARY_DIR}/../OutOfSourceDeep/deeper ABSOLUTE)

  # The maximum allowed path length on Windows is near this value.
  SET(MAXPATH "250")

  # VS8 adds "OutOfSource/SubDir/OutOfSourceSubdir/../../../" to the
  # path of the source file for no good reason.  Reduce the length
  # limit by 46 characters to account for it.  It should still be long
  # enough to require special object file name conversion.
  IF(${CMAKE_GENERATOR} MATCHES "Visual Studio 8")
    MATH(EXPR MAXPATH "${MAXPATH} - 46")
  ENDIF(${CMAKE_GENERATOR} MATCHES "Visual Studio 8")

  # MAXPATH less 25 for last /and/deeper/simple.cxx part and small safety
  MATH(EXPR MAXPATH "${MAXPATH} - 25")
  STRING(LENGTH "${DEEPDIR}" DEEPDIR_LEN)
  WHILE("${DEEPDIR_LEN}" LESS "${MAXPATH}")
    SET(DEEPDIR ${DEEPDIR}/and/deeper)
    STRING(LENGTH "${DEEPDIR}" DEEPDIR_LEN)
  ENDWHILE("${DEEPDIR_LEN}" LESS "${MAXPATH}")
  SET(DEEPSRC ${DEEPDIR}/simple.cxx)
  STRING(LENGTH "${DEEPSRC}" DEEPSRC_LEN)
  CONFIGURE_FILE(simple.cxx.in ${DEEPSRC} COPYONLY)

  # Watcom WMake seems to have problems with long command lines.  Just
  # disable this part of the test until it is resolved.
  IF(${CMAKE_GENERATOR} MATCHES "Watcom WMake")
    SET(DEEPSRC "")
    ADD_DEFINITIONS(-DNO_DEEPSRC)
  ENDIF(${CMAKE_GENERATOR} MATCHES "Watcom WMake")

  ADD_LIBRARY(testlib testlib.cxx)
  ADD_EXECUTABLE (simple simple.cxx ../simple.cxx ${DEEPSRC})
  TARGET_LINK_LIBRARIES(simple testlib outlib)
ENDIF ("${PROJECT_SOURCE_DIR}" STREQUAL "${ANOTHER_PROJ_SOURCE_DIR}")

# test getting a definition from a subdir
SET (WEASELS SIZZLING)

GET_DIRECTORY_PROPERTY(incDirs INCLUDE_DIRECTORIES)
IF(NOT incDirs)
   MESSAGE(FATAL_ERROR "GET_DIRECTORY_PROPERTY(INCLUDE_DIRECTORIES) returned empty list")
ENDIF(NOT incDirs)
