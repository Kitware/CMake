PROJECT (ExternalOBJ)

# Build the external object file.
TRY_COMPILE(EXTERNAL_OBJECT_BUILT
  ${ExternalOBJ_BINARY_DIR}/Object
  ${ExternalOBJ_SOURCE_DIR}/Object
  Object
  external
  OUTPUT_VARIABLE OUTPUT
  )
SET(EXTERNAL_OBJECT_NAME "external_object${CMAKE_CXX_OUTPUT_EXTENSION}")
IF(EXTERNAL_OBJECT_BUILT)
  MESSAGE(
    "Building ${EXTERNAL_OBJECT_NAME} succeeded with the following output:\n"
    "[${OUTPUT}]"
    )
ELSE(EXTERNAL_OBJECT_BUILT)
  MESSAGE(FATAL_ERROR
    "Building ${EXTERNAL_OBJECT_NAME} failed with the following output:\n"
    "[${OUTPUT}]"
    )
ENDIF(EXTERNAL_OBJECT_BUILT)

# Find the external object file.
SET(BASE ${ExternalOBJ_BINARY_DIR}/Object)
SET(EXTERNAL_OBJECT)
FOREACH(dir external.dir . ${CMAKE_CONFIGURATION_TYPES})
  IF(NOT EXTERNAL_OBJECT)
    IF(EXISTS ${BASE}/${dir}/${EXTERNAL_OBJECT_NAME})
      SET(EXTERNAL_OBJECT ${BASE}/${dir}/${EXTERNAL_OBJECT_NAME})
    ENDIF(EXISTS ${BASE}/${dir}/${EXTERNAL_OBJECT_NAME})
  ENDIF(NOT EXTERNAL_OBJECT)
ENDFOREACH(dir)
IF(EXTERNAL_OBJECT)
  MESSAGE("Found \"${EXTERNAL_OBJECT}\".")
ELSE(EXTERNAL_OBJECT)
  MESSAGE(FATAL_ERROR "Could not find ${EXTERNAL_OBJECT_NAME}.")
ENDIF(EXTERNAL_OBJECT)

# Build an executable using the external object file.
ADD_EXECUTABLE(executable executable.cxx ${EXTERNAL_OBJECT})
