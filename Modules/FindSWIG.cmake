# - Find SWIG
# This module finds an installed SWIG.  It sets the following variables:
#  SWIG_FOUND - set to true if SWIG is found
#  SWIG_DIR - the directory where swig is installed
#  SWIG_EXECUTABLE - the path to the swig executable

SET(SWIG_FOUND FOOBAR)
FIND_PATH(SWIG_DIR
  SWIGConfig.cmake
  /usr/share/swig1.3
  /usr/lib/swig1.3
  /usr/local/share/swig1.3)
FIND_PATH(SWIG_DIR
  swig.swg
  /usr/share/swig1.3
  /usr/lib/swig1.3
  /usr/local/share/swig1.3)
IF(EXISTS ${SWIG_DIR})
  IF("x${SWIG_DIR}x" STREQUAL "x${CMAKE_ROOT}/Modulesx")
    MESSAGE("SWIG_DIR should not be modules subdirectory of CMake")
  ENDIF("x${SWIG_DIR}x" STREQUAL "x${CMAKE_ROOT}/Modulesx")

  IF(EXISTS ${SWIG_DIR}/SWIGConfig.cmake)
    INCLUDE(${SWIG_DIR}/SWIGConfig.cmake)
    SET(SWIG_FOUND 1)
  ELSE(EXISTS ${SWIG_DIR}/SWIGConfig.cmake)
    FIND_PROGRAM(SWIG_EXECUTABLE
      NAMES swig-1.3 swig
      PATHS ${SWIG_DIR} ${SWIG_DIR}/.. ${SWIG_DIR}/../../bin /usr/bin /usr/local/bin )
    SET(SWIG_USE_FILE ${CMAKE_ROOT}/Modules/UseSWIG.cmake)
  ENDIF(EXISTS ${SWIG_DIR}/SWIGConfig.cmake)
ENDIF(EXISTS ${SWIG_DIR})

IF("x${SWIG_FOUND}x" STREQUAL "xFOOBARx")
  SET(SWIG_FOUND 0)
  IF(EXISTS ${SWIG_DIR})
    IF(EXISTS ${SWIG_USE_FILE})
      IF(EXISTS ${SWIG_EXECUTABLE})
        SET(SWIG_FOUND 1)
      ENDIF(EXISTS ${SWIG_EXECUTABLE})
    ENDIF(EXISTS ${SWIG_USE_FILE})
  ENDIF(EXISTS ${SWIG_DIR})
  IF(NOT ${SWIG_FOUND})
    IF(${SWIG_FIND_REQUIRED})
      MESSAGE(FATAL_ERROR "Swig was not found on the system. Please specify the location of Swig.")
    ENDIF(${SWIG_FIND_REQUIRED})
  ENDIF(NOT ${SWIG_FOUND})
ENDIF("x${SWIG_FOUND}x" STREQUAL "xFOOBARx")
