#
# Check if the type exists and determine size of type.  if the type
# exists, the size will be stored to the variable.
#
# CHECK_TYPE_SIZE - macro which checks the size of type
# VARIABLE - variable to store size if the type exists.
#

MACRO(CHECK_TYPE_SIZE TYPE VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(MACRO_CHECK_TYPE_SIZE_FLAGS -DCHECK_TYPE_SIZE_TYPE="${TYPE}")
    IF(HAVE_SYS_TYPES_H)
      SET(MACRO_CHECK_TYPE_SIZE_FLAGS 
          "${MACRO_CHECK_TYPE_SIZE_FLAGS} -DHAVE_SYS_TYPES_H")
    ENDIF(HAVE_SYS_TYPES_H)
    IF(HAVE_STDINT_H)
      SET(MACRO_CHECK_TYPE_SIZE_FLAGS "${MACRO_CHECK_TYPE_SIZE_FLAGS} -DHAVE_STDINT_H")
    ENDIF(HAVE_STDINT_H)
    MESSAGE(STATUS "Check size of ${TYPE}")
    IF(CMAKE_REQUIRED_LIBRARIES)
      SET(CHECK_TYPE_SIZE_ADD_LIBRARIES 
          "-DLINK_LIBRARIES:STRING=${CMAKE_REQUIRED_LIBRARIES}")
    ENDIF(CMAKE_REQUIRED_LIBRARIES)
    TRY_RUN(${VARIABLE} HAVE_${VARIABLE}
            ${CMAKE_BINARY_DIR}
            ${CMAKE_ROOT}/Modules/CheckTypeSize.c
            CMAKE_FLAGS -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_TYPE_SIZE_FLAGS}
                        "${CHECK_TYPE_SIZE_ADD_LIBRARIES}"
            OUTPUT_VARIABLE OUTPUT)
    IF(HAVE_${VARIABLE})
      MESSAGE(STATUS "Check size of ${TYPE} - done")
    ELSE(HAVE_${VARIABLE})
      MESSAGE(STATUS "Check size of ${TYPE} - failed")
      WRITE_FILE(${CMAKE_BINARY_DIR}/CMakeError.log 
        "Determining size of ${TYPE} failed with the following output:\n${OUTPUT}\n"
        APPEND)
    ENDIF(HAVE_${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_TYPE_SIZE)
