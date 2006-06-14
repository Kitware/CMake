# - Define macro to determine endian type
# Check if the system is big endian or little endian
#  TEST_BIG_ENDIAN(VARIABLE)
#  VARIABLE - variable to store the result to
#

MACRO(TEST_BIG_ENDIAN VARIABLE)
  IF("HAVE_${VARIABLE}" MATCHES "^HAVE_${VARIABLE}$")
    TRY_RUN(${VARIABLE} HAVE_${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_ROOT}/Modules/TestBigEndian.c
      OUTPUT_VARIABLE OUTPUT)
    IF("${VARIABLE}" STREQUAL "FAILED_TO_RUN")
      MESSAGE(SEND_ERROR "TestBigEndian Failed to run with output: ${OUTPUT}")
    ENDIF("${VARIABLE}" STREQUAL "FAILED_TO_RUN")
    MESSAGE(STATUS "Check if the system is big endian")
    IF(HAVE_${VARIABLE})
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
        "Determining the endianes of the system passed. The system is ")
      IF(${VARIABLE})
        FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
          "big endian")
        MESSAGE(STATUS "Check if the system is big endian - big endian")
      ELSE(${VARIABLE})
        FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
          "little endian")
        MESSAGE(STATUS "Check if the system is big endian - little endian")
      ENDIF(${VARIABLE})
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Test produced following output:\n${OUTPUT}\n\n")
    ELSE(HAVE_${VARIABLE})
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
        "Determining the endianes of the system failed with the following output:\n${OUTPUT}\n\n")
      MESSAGE("Check if the system is big endian - failed")
    ENDIF(HAVE_${VARIABLE})
  ENDIF("HAVE_${VARIABLE}" MATCHES "^HAVE_${VARIABLE}$")
ENDMACRO(TEST_BIG_ENDIAN)
