# - Test for compiler support of ANSI stream headers iostream, etc.
# check if we they have the standard ansi stream files (without the .h)
#  CMAKE_NO_ANSI_STREAM_HEADERS - defined by the results
#
INCLUDE(CheckIncludeFileCXX)

IF(NOT CMAKE_NO_ANSI_STREAM_HEADERS)
  CHECK_INCLUDE_FILE_CXX(iostream CMAKE_ANSI_STREAM_HEADERS)
  IF (CMAKE_ANSI_STREAM_HEADERS)
    SET (CMAKE_NO_ANSI_STREAM_HEADERS 0 CACHE INTERNAL 
         "Does the compiler support headers like iostream.")
  ELSE (CMAKE_ANSI_STREAM_HEADERS)   
    SET (CMAKE_NO_ANSI_STREAM_HEADERS 1 CACHE INTERNAL 
       "Does the compiler support headers like iostream.")
  ENDIF (CMAKE_ANSI_STREAM_HEADERS)

  MARK_AS_ADVANCED(CMAKE_NO_ANSI_STREAM_HEADERS)
ENDIF(NOT CMAKE_NO_ANSI_STREAM_HEADERS)


