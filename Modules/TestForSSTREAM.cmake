# # - Test for std:: namespace support
# check if the compiler supports std:: on stl classes
#  CMAKE_NO_STD_NAMESPACE - defined by the results
#
IF("CMAKE_HAS_ANSI_STRING_STREAM" MATCHES "^CMAKE_HAS_ANSI_STRING_STREAM$")
  MESSAGE(STATUS "Check for sstream")
  TRY_COMPILE(CMAKE_HAS_ANSI_STRING_STREAM  ${CMAKE_BINARY_DIR} 
    ${CMAKE_ROOT}/Modules/TestForSSTREAM.cxx
    OUTPUT_VARIABLE OUTPUT)
  IF (CMAKE_HAS_ANSI_STRING_STREAM)
    MESSAGE(STATUS "Check for sstream - found")
    SET (CMAKE_NO_ANSI_STRING_STREAM 0 CACHE INTERNAL 
         "Does the compiler support sstream")
    FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if the CXX compiler has sstream passed with "
      "the following output:\n${OUTPUT}\n\n")
  ELSE (CMAKE_HAS_ANSI_STRING_STREAM)
    MESSAGE(STATUS "Check for sstream - not found")
    SET (CMAKE_NO_ANSI_STRING_STREAM 1 CACHE INTERNAL 
       "Does the compiler support sstream")
    FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
      "Determining if the CXX compiler has sstream failed with "
      "the following output:\n${OUTPUT}\n\n")
  ENDIF (CMAKE_HAS_ANSI_STRING_STREAM)
ENDIF("CMAKE_HAS_ANSI_STRING_STREAM" MATCHES "^CMAKE_HAS_ANSI_STRING_STREAM$")




