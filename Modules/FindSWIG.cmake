SET(SWIG_FOUND FOOBAR)
FIND_PATH(SWIG_DIR
  SWIGConfig.cmake
  /usr/share/swig1.3
  /usr/local/share/swig1.3)
FIND_PATH(SWIG_DIR
  swig.swg
  /usr/share/swig1.3
  /usr/local/share/swig1.3)
IF(EXISTS ${SWIG_DIR})
  IF("x${SWIG_DIR}x" MATCHES "^x${CMAKE_ROOT}/Modulesx$")
    MESSAGE("SWIG_DIR should not be modules subdirectory of CMake")
  ENDIF("x${SWIG_DIR}x" MATCHES "^x${CMAKE_ROOT}/Modulesx$")

  IF(EXISTS ${SWIG_DIR}/SWIGConfig.cmake)
    INCLUDE(${SWIG_DIR}/SWIGConfig.cmake)
  ELSE(EXISTS ${SWIG_DIR}/SWIGConfig.cmake)
    FIND_PROGRAM(SWIG_EXECUTABLE
      NAMES swig-1.3 swig
      PATHS ${SWIG_DIR} /usr/bin /usr/local/bin )
    SET(SWIG_USE_FILE ${CMAKE_ROOT}/Modules/UseSWIG.cmake)
  ENDIF(EXISTS ${SWIG_DIR}/SWIGConfig.cmake)
ENDIF(EXISTS ${SWIG_DIR})

IF("x${SWIG_FOUND}x" MATCHES "^xFOOBARx$")
  SET(SWIG_FOUND 0)
  IF(SWIG_DIR)
    IF(EXISTS ${SWIG_USE_FILE})
      IF(EXISTS ${SWIG_EXECUTABLE})
        SET(SWIG_FOUND 1)
      ENDIF(EXISTS ${SWIG_EXECUTABLE})
    ENDIF(EXISTS ${SWIG_USE_FILE})
  ENDIF(SWIG_DIR)
ENDIF("x${SWIG_FOUND}x" MATCHES "^xFOOBARx$")

