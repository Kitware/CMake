#
# Check if the type exists and determine size of type.  if the type
# exists, the size will be stored to the variable.
#
# CHECK_TYPE_SIZE - macro which checks the size of type
# VARIABLE - variable to store size if the type exists.
#

MACRO(CHECK_TYPE_SIZE TYPE VARIABLE)
  TRY_RUN(${VARIABLE} HAVE_${VARIABLE}
          ${PROJECT_BINARY_DIR}
          ${CMAKE_ROOT}/Modules/CheckSizeOf.c
          COMPILE_DEFINITIONS -DCHECK_SIZE_OF="${TYPE}"
          OUTPUT_VARIABLE OUTPUT)
  IF(NOT HAVE_${VARIABLE})
    WRITE_FILE(${PROJECT_BINARY_DIR}/CMakeError.log 
      "Determining size of ${TYPE} failed with the following output:\n${OUTPUT}\n")
  ENDIF(NOT HAVE_${VARIABLE})
ENDMACRO(CHECK_TYPE_SIZE)
