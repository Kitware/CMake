cmake_minimum_required (VERSION 2.6)
PROJECT(SUBDIR)

# Some systems do not seem to support rpath with spaces.
IF("${CMAKE_SYSTEM}" MATCHES "IRIX|QNX")
  SET(CMAKE_SKIP_BUILD_RPATH 1)
ENDIF("${CMAKE_SYSTEM}" MATCHES "IRIX|QNX")

# be able to see output from make on dashboards
SET(CMAKE_VERBOSE_MAKEFILE 1)
message("${CMAKE_MAKE_PROGRAM}")
set(CMAKE_PAREN TRUE)
IF("${CMAKE_MAKE_PROGRAM}" MATCHES "wmake")
  message("wmake does not support () in path")
  set(CMAKE_PAREN FALSE)
ENDIF("${CMAKE_MAKE_PROGRAM}" MATCHES "wmake")

IF(CMAKE_PAREN)
  ADD_DEFINITIONS(-DCMAKE_PAREN=1)
  SUBDIRS("Executable Sources" "Some(x86) Sources" EXCLUDE_FROM_ALL "Some Examples")
ELSE(CMAKE_PAREN)
  SUBDIRS("Executable Sources" EXCLUDE_FROM_ALL "Some Examples")
ENDIF(CMAKE_PAREN)

WRITE_FILE(${SUBDIR_BINARY_DIR}/ShouldBeHere "This file should exist.")
#WATCOM WMAKE does not support + in the name of a file!
IF(WATCOM)
  SET(PLUS_NAME_FILES 
    "Another Subdir/pair_int.int.c"
    vcl_algorithm_vcl_pair_double.foo.c)
ELSE(WATCOM)
    SET(PLUS_NAME_FILES 
    "Another Subdir/pair+int.int.c"
    vcl_algorithm+vcl_pair+double.foo.c)
ENDIF(WATCOM)

ADD_EXECUTABLE(TestFromSubdir
  "Another Subdir/testfromsubdir.c"
  "Another Subdir/secondone"
  ${PLUS_NAME_FILES}
  )

AUX_SOURCE_DIRECTORY(ThirdSubDir SOURCES)
IF(WATCOM)
  FOREACH(f ${SOURCES})
    IF("${f}" STREQUAL "ThirdSubDir/pair+int.int1.c")
    ELSE("${f}" STREQUAL "ThirdSubDir/pair+int.int1.c")
      SET(SOURCES2 ${f} ${SOURCES2})
    ENDIF("${f}" STREQUAL "ThirdSubDir/pair+int.int1.c")
  ENDFOREACH(f)
  SET(SOURCES ${SOURCES2})
  SET(SOURCES ${SOURCES} 
    vcl_algorithm_vcl_pair_double.foo.c)
ELSE(WATCOM)
  FOREACH(f ${SOURCES})
    IF("${f}" STREQUAL "ThirdSubDir/pair_int.int1.c")
    ELSE("${f}" STREQUAL "ThirdSubDir/pair_int.int1.c")
      SET(SOURCES2 ${f} ${SOURCES2})
    ENDIF("${f}" STREQUAL "ThirdSubDir/pair_int.int1.c")
  ENDFOREACH(f)
  SET(SOURCES ${SOURCES2})
  SET(SOURCES ${SOURCES} 
    vcl_algorithm+vcl_pair+double.foo.c)
ENDIF(WATCOM)  
ADD_EXECUTABLE(TestWithAuxSourceDir ${SOURCES})
IF(CMAKE_PAREN)
  target_link_libraries(TestWithAuxSourceDir testOddPath)
ENDIF(CMAKE_PAREN)

