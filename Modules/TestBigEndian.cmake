#
# Check if the system is big endian or little endian
#
# VARIABLE - variable to store the result to
#

MACRO(TEST_BIG_ENDIAN VARIABLE)
  TRY_RUN(${VARIABLE} HAVE_${VARIABLE}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_ROOT}/Modules/TestBigEndian.c
    OUTPUT_VARIABLE OUTPUT)
  IF(NOT HAVE_${VARIABLE})
    FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeError.log 
      "Determining the endianes of the system failed with the following output:\n${OUTPUT}\n\n")
  ENDIF(NOT HAVE_${VARIABLE})
ENDMACRO(TEST_BIG_ENDIAN)
