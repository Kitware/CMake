#
# Check if the function exists.
#
# CHECK_LIBRARY_EXISTS - macro which checks if the function exists
# FUNCTION - the name of the function
# VARIABLE - variable to store the result
#

MACRO(CHECK_LIBRARY_EXISTS LIBRARY FUNCTION LOCATION VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(MACRO_CHECK_LIBRARY_EXISTS_DEFINITION -DCHECK_FUNCTION_EXISTS=${FUNCTION})
    MESSAGE(STATUS "Looking for ${FUNCTION} in ${LIBRARY}")
    TRY_COMPILE(${VARIABLE}
               ${PROJECT_BINARY_DIR}
               ${CMAKE_ROOT}/Modules/CheckFunctionExists.c
               CMAKE_FLAGS 
                 -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_LIBRARY_EXISTS_DEFINITION}
                 -DLINK_DIRECTORIES:STRING=${LOCATION}
                 -DLINK_LIBRARIES:STRING=${LIBRARY}
               OUTPUT_VARIABLE OUTPUT)

    IF(${VARIABLE})
      MESSAGE(STATUS "Looking for ${FUNCTION} in ${LIBRARY} - found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have library ${LIBRARY}")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Looking for ${FUNCTION} in ${LIBRARY} - not found")
      SET(${VARIABLE} "" CACHE INTERNAL "Have library ${LIBRARY}")
      WRITE_FILE(${PROJECT_BINARY_DIR}/CMakeError.log 
        "Determining if the function ${FUNCTION} exists in the ${LIBRARY} "
        "failed with the following output:\n"
        "${OUTPUT}\n" APPEND)
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_LIBRARY_EXISTS)
