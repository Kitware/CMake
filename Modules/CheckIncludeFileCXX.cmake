#
# Check if the include file exists.
#
# CHECK_INCLUDE_FILE - macro which checks the include file exists.
# INCLUDE - name of include file
# VARIABLE - variable to return result
#

MACRO(CHECK_INCLUDE_FILE_CXX INCLUDE VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    MESSAGE(STATUS "Checking for CXX include file ${INCLUDE}")
    SET(CHECK_INCLUDE_FILE_VAR ${INCLUDE})
    CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CheckIncludeFile.cxx.in
                   ${CMAKE_BINARY_DIR}/CMakeTmp/CheckIncludeFile.cxx IMMEDIATE)
    TRY_COMPILE(${VARIABLE}
               ${CMAKE_BINARY_DIR}
               ${CMAKE_BINARY_DIR}/CMakeTmp/CheckIncludeFile.cxx
               CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${CMAKE_ANSI_CXXFLAGS}
               OUTPUT_VARIABLE OUTPUT)
    IF(${VARIABLE})
      MESSAGE(STATUS "Checking for CXX include file ${INCLUDE} -- found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have include ${INCLUDE}")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Checking for CXX include file ${INCLUDE} -- not found")
      SET(${VARIABLE} "" CACHE INTERNAL "Have include ${INCLUDE}")
      WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeError.log 
        "Determining if the include file ${INCLUDE} "
        "exists failed with the following output:\n"
        "${OUTPUT}\n" APPEND)
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_INCLUDE_FILE_CXX)
