PROJECT(SUBDIR)

# Some systems do not seem to support rpath with spaces.
IF("${CMAKE_SYSTEM}" MATCHES "IRIX|QNX")
  SET(CMAKE_SKIP_BUILD_RPATH 1)
ENDIF("${CMAKE_SYSTEM}" MATCHES "IRIX|QNX")

# be able to see output from make on dashboards
SET(CMAKE_VERBOSE_MAKEFILE 1)
SUBDIRS("Executable Sources" "Some(x86) Sources" EXCLUDE_FROM_ALL "Some Examples")
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
target_link_libraries(TestWithAuxSourceDir testOddPath)

