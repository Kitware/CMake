#
# check if we they have the standard ansi stream files (without the .h)
#
# CMAKE_NO_ANSI_STREAM_HEADERS - defined accoreding to the results
#

IF(NOT CMAKE_NO_ANSI_STREAM_HEADERS)
  MESSAGE(STATUS "Check for ANSI stream headers")
  TRY_COMPILE(CMAKE_ANSI_STREAM_HEADERS  ${PROJECT_BINARY_DIR} 
    ${CMAKE_ROOT}/Modules/TestForANSIStreamHeaders.cxx)
  IF (CMAKE_ANSI_STREAM_HEADERS)
    MESSAGE(STATUS "Check for ANSI stream headers - found")
    SET (CMAKE_NO_ANSI_STREAM_HEADERS 0 CACHE INTERNAL 
         "Does the compiler support headers like iostream.")
  ELSE (CMAKE_ANSI_STREAM_HEADERS)
    MESSAGE(STATUS "Check for ANSI stream headers - not found")
    SET (CMAKE_NO_ANSI_STREAM_HEADERS 1 CACHE INTERNAL 
       "Does the compiler support headers like iostream.")
  ENDIF (CMAKE_ANSI_STREAM_HEADERS)

  MARK_AS_ADVANCED(CMAKE_NO_ANSI_STREAM_HEADERS)
ENDIF(NOT CMAKE_NO_ANSI_STREAM_HEADERS)


