#
# Check if the include file exists.
#
# CHECK_INCLUDE_FILE - macro which checks the include file exists.
# INCLUDE - name of include file
# VARIABLE - variable to return result
# OPTIONAL - a third argument can be extra flags which are passed to the compiler
#

MACRO(CHECK_INCLUDE_FILE_CXX INCLUDE VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    MESSAGE(STATUS "Checking for CXX include file ${INCLUDE}")
    SET(CHECK_INCLUDE_FILE_VAR ${INCLUDE})
    CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CheckIncludeFile.cxx.in
      ${CMAKE_BINARY_DIR}/CMakeTmp/CheckIncludeFile.cxx IMMEDIATE)
    IF(${ARGC} EQUAL 2)
      SET(CMAKE_CXX_FLAGS_SAVE ${CMAKE_CXX_FLAGS})
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARGV2}")
    ENDIF(${ARGC} EQUAL 2)

    TRY_COMPILE(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}/CMakeTmp/CheckIncludeFile.cxx
      OUTPUT_VARIABLE OUTPUT)

    IF(${ARGC} EQUAL 2)
      SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_SAVE})
    ENDIF(${ARGC} EQUAL 2)

    IF(${VARIABLE})
      MESSAGE(STATUS "Checking for CXX include file ${INCLUDE} -- found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have include ${INCLUDE}")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeOutput.log 
        "Determining if the include file ${INCLUDE} "
        "exists passed with the following output:\n"
        "${OUTPUT}\n\n")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Checking for CXX include file ${INCLUDE} -- not found")
      SET(${VARIABLE} "" CACHE INTERNAL "Have include ${INCLUDE}")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log 
        "Determining if the include file ${INCLUDE} "
        "exists failed with the following output:\n"
        "${OUTPUT}\n\n")
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_INCLUDE_FILE_CXX)
