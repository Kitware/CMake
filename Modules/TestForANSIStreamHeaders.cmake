#
# check if we they have the standard ansi stream files (without the .h)
#
# CMAKE_NO_ANSI_STREAM_HEADERS - defined accoreding to the results
#

MESSAGE(${CMAKE_MAKE_PROGRAM})
MESSAGE(${PROJECT_BINARY_DIR})
TRY_COMPILE(CMAKE_ANSI_STREAM_HEADERS ${CMAKE_ROOT}/Modules/TestForANSIStreamHeaders.cxx ${PROJECT_BINARY_DIR})
IF (CMAKE_ANSI_STREAM_HEADERS)
  SET (CMAKE_NO_ANSI_STREAM_HEADERS 0 CACHE INTERNAL 
       "Does the compiler support headers like iostream.")
ELSE (CMAKE_ANSI_STREAM_HEADERS)
  SET (CMAKE_NO_ANSI_STREAM_HEADERS 1 CACHE INTERNAL 
     "Does the compiler support headers like iostream.")
ENDIF (CMAKE_ANSI_STREAM_HEADERS)

MARK_AS_ADVANCED(CMAKE_NO_ANSI_STREAM_HEADERS)


