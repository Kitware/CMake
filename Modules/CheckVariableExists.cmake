#
# Check if the variable exists.
# # CHECK_VARIABLE_EXISTS - macro which checks if the variable exists
# VAR - the name of the variable
# VARIABLE - variable to store the result
#

MACRO(CHECK_VARIABLE_EXISTS VAR VARIABLE)
  SET(MACRO_CHECK_VARIABLE_DEFINITIONS -DCHECK_VARIABLE_EXISTS=${VAR})
  TRY_COMPILE(${VARIABLE}
             ${PROJECT_BINARY_DIR}
             ${CMAKE_ROOT}/Modules/CheckVariableExists.c
             CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_VARIABLE_DEFINITIONS}
             OUTPUT_VARIABLE OUTPUT)
  IF(${VARIABLE})
    SET(${VARIABLE} 1 CACHE INTERNAL "Have variable ${VAR}")
  ELSE(${VARIABLE})
    SET(${VARIABLE} "" CACHE INTERNAL "Have variable ${VAR}")
    WRITE_FILE(${PROJECT_BINARY_DIR}/CMakeError.log 
      "Determining if the variable ${VAR} exists failed with the following output:\n"
      "${OUTPUT}\n" APPEND)
  ENDIF(${VARIABLE})
ENDMACRO(CHECK_VARIABLE_EXISTS)
