#
# Check if the type exists and determine size of type.  if the type
# exists, the size will be stored to the variable.
#
# CHECK_TYPE_SIZE - macro which checks the size of type
# VARIABLE - variable to store size if the type exists.
#

MACRO(CHECK_TYPE_SIZE TYPE VARIABLE)
  SET(MACRO_CHECK_TYPE_SIZE_FLAGS -DCHECK_SIZE_OF="${TYPE}")
  IF(HAVE_SYS_TYPES_H)
    SET(MACRO_CHECK_TYPE_SIZE_FLAGS "${MACRO_CHECK_TYPE_SIZE_FLAGS} -DHAVE_SYS_TYPES_H")
  ENDIF(HAVE_SYS_TYPES_H)
  IF(HAVE_STDINT_H)
    SET(MACRO_CHECK_TYPE_SIZE_FLAGS "${MACRO_CHECK_TYPE_SIZE_FLAGS} -DHAVE_STDINT_H")
  ENDIF(HAVE_STDINT_H)

  TRY_RUN(${VARIABLE} HAVE_${VARIABLE}
          ${PROJECT_BINARY_DIR}
          ${CMAKE_ROOT}/Modules/CheckSizeOf.c
          CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_TYPE_SIZE_FLAGS}
          OUTPUT_VARIABLE OUTPUT)
  IF(NOT HAVE_${VARIABLE})
    WRITE_FILE(${PROJECT_BINARY_DIR}/CMakeError.log 
      "Determining size of ${TYPE} failed with the following output:\n${OUTPUT}\n")
  ENDIF(NOT HAVE_${VARIABLE})
ENDMACRO(CHECK_TYPE_SIZE)
