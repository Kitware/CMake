#
# Check if the type exists and determine size of type.  if the type
# exists, the size will be stored to the variable.
#
# CHECK_TYPE_SIZE - macro which checks the size of type
# VARIABLE - variable to store size if the type exists.
#

MACRO(CHECK_TYPE_SIZE TYPE VARIABLE)
  TRY_RUN(RUN_RESULT COMPILE_OK 
          ${PROJECT_BINARY_DIR}
          ${CMAKE_ROOT}/Modules/CheckSizeOf.c
          COMPILE_DEFINITIONS -DCHECK_SIZE_OF="${TYPE}"
          OUTPUT_VARIABLE OUTPUT)
  IF(COMPILE_OK)
    SET(${VARIABLE} ${RUN_RESULT})
  ELSE(COMPILE_OK)
    WRITE_FILE(${PROJECT_BINARY_DIR}/CMakeError.log 
      "Determining size of ${TYPE} failed with the following output:\n${OUTPUT}\n")
  ENDIF(COMPILE_OK)
ENDMACRO(CHECK_TYPE_SIZE)
