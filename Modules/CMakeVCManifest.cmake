
# Leave the first line of this file empty so this module will not be
# included in the documentation.

# This script is invoked from Windows-cl.cmake and passed the TARGET
# variable on the command line.

# Conditionally embed the manifest in the executable if it exists.
IF(EXISTS "${TARGET}.manifest")
  # Construct the manifest embedding command.
  SET(CMD
    mt ${CMAKE_CL_NOLOGO} /manifest ${TARGET}.manifest
    /outputresource:${TARGET}
    )

  # Run the embedding command.
  EXECUTE_PROCESS(COMMAND ${CMD}\;\#2 RESULT_VARIABLE RESULT)

  # Check whether the command failed.
  IF(NOT "${RESULT}" MATCHES "^0$")
    # The embedding failed remove the target and the manifest.
    FILE(REMOVE ${TARGET} ${TARGET}.manifest)

    # Describe the failure in a message.
    STRING(REGEX REPLACE ";" " " CMD "${CMD}")
    MESSAGE(FATAL_ERROR
      "Failed to embed manifest in ${TARGET} using command \"${CMD};#2\""
      )
  ENDIF(NOT "${RESULT}" MATCHES "^0$")
ENDIF(EXISTS "${TARGET}.manifest")
