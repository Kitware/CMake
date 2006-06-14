# - Check if the variable exists.
#  CHECK_VARIABLE_EXISTS(VAR VARIABLE)
#  
#  VAR      - the name of the variable
#  VARIABLE - variable to store the result
#
# This macro is only for C variables.
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_LIBRARIES = list of libraries to link

MACRO(CHECK_VARIABLE_EXISTS VAR VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(MACRO_CHECK_VARIABLE_DEFINITIONS 
      "-DCHECK_VARIABLE_EXISTS=${VAR} ${CMAKE_REQUIRED_FLAGS}")
    MESSAGE(STATUS "Looking for ${VAR}")
    IF(CMAKE_REQUIRED_LIBRARIES)
      SET(CHECK_VARIABLE_EXISTS_ADD_LIBRARIES
        "-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
    ELSE(CMAKE_REQUIRED_LIBRARIES)
      SET(CHECK_VARIABLE_EXISTS_ADD_LIBRARIES)
    ENDIF(CMAKE_REQUIRED_LIBRARIES)
    TRY_COMPILE(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_ROOT}/Modules/CheckVariableExists.c
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_VARIABLE_DEFINITIONS}
      "${CHECK_VARIABLE_EXISTS_ADD_LIBRARIES}"
      OUTPUT_VARIABLE OUTPUT)
    IF(${VARIABLE})
      SET(${VARIABLE} 1 CACHE INTERNAL "Have variable ${VAR}")
      MESSAGE(STATUS "Looking for ${VAR} - found")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log 
        "Determining if the variable ${VAR} exists passed with the following output:\n"
        "${OUTPUT}\n\n")
    ELSE(${VARIABLE})
      SET(${VARIABLE} "" CACHE INTERNAL "Have variable ${VAR}")
      MESSAGE(STATUS "Looking for ${VAR} - not found")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
        "Determining if the variable ${VAR} exists failed with the following output:\n"
        "${OUTPUT}\n\n")
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_VARIABLE_EXISTS)
