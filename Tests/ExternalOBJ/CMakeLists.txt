cmake_minimum_required (VERSION 2.6)
PROJECT (ExternalOBJ)

IF(APPLE)
  # set _CMAKE_OSX_MACHINE to umame -m
  EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE _CMAKE_OSX_MACHINE)
  # check for Power PC and change to ppc
  IF("${_CMAKE_OSX_MACHINE}" MATCHES "Power")
    SET(_CMAKE_OSX_MACHINE ppc)
  ENDIF("${_CMAKE_OSX_MACHINE}" MATCHES "Power")
  SET(CMAKE_OSX_ARCHITECTURES ${_CMAKE_OSX_MACHINE})
ENDIF(APPLE)

# Build the external object file.
TRY_COMPILE(EXTERNAL_OBJECT_BUILT
  ${ExternalOBJ_BINARY_DIR}/Object
  ${ExternalOBJ_SOURCE_DIR}/Object
  Object
  external
  OUTPUT_VARIABLE OUTPUT
  )
IF(EXTERNAL_OBJECT_BUILT)
  MESSAGE(
    "Building external_object.cxx succeeded with the following output:\n"
    "[${OUTPUT}]"
    )
ELSE(EXTERNAL_OBJECT_BUILT)
  MESSAGE(FATAL_ERROR
    "Building external_object.cxx failed with the following output:\n"
    "[${OUTPUT}]"
    )
ENDIF(EXTERNAL_OBJECT_BUILT)

# Find the external object file.
SET(DIR ${ExternalOBJ_BINARY_DIR}/Object)
FILE(GLOB_RECURSE EXTERNAL_OBJECT
  "${DIR}/external_object*${CMAKE_CXX_OUTPUT_EXTENSION}")
IF(EXTERNAL_OBJECT)
  LIST (GET EXTERNAL_OBJECT 0 EXTERNAL_OBJECT)
  MESSAGE("Found \"${EXTERNAL_OBJECT}\".")
ELSE(EXTERNAL_OBJECT)
  MESSAGE(FATAL_ERROR "Could not find external object.")
ENDIF(EXTERNAL_OBJECT)

# Test creation of external objects by custom commands.
SET(CUSTOM_OBJECT
  ${CMAKE_CURRENT_BINARY_DIR}/custom_object${CMAKE_C_OUTPUT_EXTENSION})
ADD_CUSTOM_COMMAND(
  OUTPUT ${CUSTOM_OBJECT}
  COMMAND ${CMAKE_COMMAND} -E copy ${EXTERNAL_OBJECT} ${CUSTOM_OBJECT}
  DEPENDS ${EXTERNAL_OBJECT}
  )

message("${EXTERNAL_OBJECT}")
# Build an executable using the external object file.
ADD_EXECUTABLE(ExternalOBJ executable.cxx ${CUSTOM_OBJECT})
# A bug showed up in VS2010 where an object file that was
# part of a custom commad output worked, but ones that were
# not didn't work.  So, repeat the executable using the object
# directly and not from the output of the copy.
ADD_EXECUTABLE(ExternalOBJ2 executable.cxx ${EXTERNAL_OBJECT})
