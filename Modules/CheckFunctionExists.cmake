#
# Check if the type exists and determine size of type.  if the type
# exists, the size will be stored to the variable.
#
# CHECK_TYPE_SIZE - macro which checks the size of type
# VARIABLE - variable to store size if the type exists.
#

MACRO(CHECK_FUNCTION_EXISTS FUNCTION VARIABLE)
  TRY_COMPILE(COMPILE_OK
             ${PROJECT_BINARY_DIR}
             ${CMAKE_ROOT}/Modules/CheckFunctionExists.c
             COMPILE_DEFINITIONS -DCHECK_FUNCTION_EXISTS=${FUNCTION} 
             OUTPUT_VARIABLE OUTPUT)
  IF(COMPILE_OK)
    SET(${VARIABLE} ${COMPILE_OK})
  ELSE(COMPILE_OK)
    WRITE_FILE(${PROJECT_BINARY_DIR}/CMakeError.log 
      "Determining if the function ${FUNCTION} exists failed with the following output:\n"
      "${OUTPUT}\n")
  ENDIF(COMPILE_OK)
ENDMACRO(CHECK_FUNCTION_EXISTS)
