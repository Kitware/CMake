cmake_minimum_required (VERSION 2.6)
PROJECT(SUBDIR)
SUBDIRS(Executable EXCLUDE_FROM_ALL Examples)
WRITE_FILE(${SUBDIR_BINARY_DIR}/ShouldBeHere "This file should exist.")
#WATCOM WMAKE does not support + in the name of a file!
IF(WATCOM)
  SET(PLUS_NAME_FILES 
    AnotherSubdir/pair_int.int.c
    vcl_algorithm_vcl_pair_double.foo.c)
ELSE(WATCOM)
    SET(PLUS_NAME_FILES 
    AnotherSubdir/pair+int.int.c
    vcl_algorithm+vcl_pair+double.foo.c)
ENDIF(WATCOM)

ADD_EXECUTABLE(TestFromSubdir
  AnotherSubdir/testfromsubdir.c
  AnotherSubdir/secondone
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
      MESSAGE("${f}")
    ENDIF("${f}" STREQUAL "ThirdSubDir/pair_int.int1.c")
  ENDFOREACH(f)
  SET(SOURCES ${SOURCES2})
  SET(SOURCES ${SOURCES} 
    vcl_algorithm+vcl_pair+double.foo.c)
ENDIF(WATCOM)  
MESSAGE("Sources: ${SOURCES}")
ADD_EXECUTABLE(TestWithAuxSourceDir ${SOURCES})
