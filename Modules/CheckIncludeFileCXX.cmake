# - Check if the include file exists.
#  CHECK_INCLUDE_FILE_CXX(INCLUDE VARIABLE)
#
#  INCLUDE  - name of include file
#  VARIABLE - variable to return result
#  
# An optional third argument is the CFlags to add to the compile line 
# or you can use CMAKE_REQUIRED_FLAGS.
#
MACRO(CHECK_INCLUDE_FILE_CXX INCLUDE VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(MACRO_CHECK_INCLUDE_FILE_FLAGS ${CMAKE_REQUIRED_FLAGS})
    SET(CHECK_INCLUDE_FILE_VAR ${INCLUDE})
    CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CheckIncludeFile.cxx.in
      ${CMAKE_BINARY_DIR}/CMakeTmp/CheckIncludeFile.cxx IMMEDIATE)
    MESSAGE(STATUS "Looking for C++ include ${INCLUDE}")
    IF(${ARGC} EQUAL 3)
      SET(CMAKE_CXX_FLAGS_SAVE ${CMAKE_CXX_FLAGS})
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARGV2}")
    ENDIF(${ARGC} EQUAL 3)

    TRY_COMPILE(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}/CMakeTmp/CheckIncludeFile.cxx
      CMAKE_FLAGS 
      -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_INCLUDE_FILE_FLAGS}
      OUTPUT_VARIABLE OUTPUT) 

    IF(${ARGC} EQUAL 3)
      SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_SAVE})
    ENDIF(${ARGC} EQUAL 3)

    IF(${VARIABLE})
      MESSAGE(STATUS "Looking for C++ include ${INCLUDE} - found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have include ${INCLUDE}")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log 
        "Determining if the include file ${INCLUDE} "
        "exists passed with the following output:\n"
        "${OUTPUT}\n\n")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Looking for C++ include ${INCLUDE} - not found")
      SET(${VARIABLE} "" CACHE INTERNAL "Have include ${INCLUDE}")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log 
        "Determining if the include file ${INCLUDE} "
        "exists failed with the following output:\n"
        "${OUTPUT}\n\n")
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_INCLUDE_FILE_CXX)
