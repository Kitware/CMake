#
# Check if the function exists.
#
# CHECK_FUNCTION_EXISTS - macro which checks if the function exists
# FUNCTION - the name of the function
# VARIABLE - variable to store the result
#

MACRO(CHECK_FUNCTION_EXISTS FUNCTION VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(MACRO_CHECK_FUNCTION_DEFINITIONS -DCHECK_FUNCTION_EXISTS=${FUNCTION})
    MESSAGE(STATUS "Looking for ${FUNCTION}")
    TRY_COMPILE(${VARIABLE}
            ${PROJECT_BINARY_DIR}
            ${CMAKE_ROOT}/Modules/CheckFunctionExists.c
            CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_FUNCTION_DEFINITIONS}
            OUTPUT_VARIABLE OUTPUT)
    IF(${VARIABLE})
      SET(${VARIABLE} 1 CACHE INTERNAL "Have function ${FUNCTION}")
      MESSAGE(STATUS "Looking for ${FUNCTION} - found")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Looking for ${FUNCTION} - not found")
      SET(${VARIABLE} "" CACHE INTERNAL "Have function ${FUNCTION}")
      WRITE_FILE(${PROJECT_BINARY_DIR}/CMakeError.log 
        "Determining if the function ${FUNCTION} exists failed with the following output:\n"
        "${OUTPUT}\n" APPEND)
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_FUNCTION_EXISTS)
