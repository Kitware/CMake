#
# Check if the system is big endian or little endian
#
# VARIABLE - variable to store the result to
#

MACRO(TEST_BIG_ENDIAN VARIABLE)
  IF("HAVE_${VARIABLE}" MATCHES "^HAVE_${VARIABLE}$")
    TRY_RUN(${VARIABLE} HAVE_${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_ROOT}/Modules/TestBigEndian.c
      OUTPUT_VARIABLE OUTPUT)
    MESSAGE(STATUS "Check if the system is big endian")
    IF(HAVE_${VARIABLE})
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log 
        "Determining the endianes of the system passed. The system is ")
      IF(${VARIABLE})
        FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log 
          "big endian")
        MESSAGE(STATUS "Check if the system is big endian - big endian")
      ELSE(${VARIABLE})
        FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log 
          "little endian")
        MESSAGE(STATUS "Check if the system is big endian - little endian")
      ENDIF(${VARIABLE})
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log
        "Test produced following output:\n${OUTPUT}\n\n")
    ELSE(HAVE_${VARIABLE})
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log 
        "Determining the endianes of the system failed with the following output:\n${OUTPUT}\n\n")
      MESSAGE("Check if the system is big endian - failed")
    ENDIF(HAVE_${VARIABLE})
  ENDIF("HAVE_${VARIABLE}" MATCHES "^HAVE_${VARIABLE}$")
ENDMACRO(TEST_BIG_ENDIAN)
