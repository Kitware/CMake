PROJECT(SUBDIR)
SUBDIRS(Executable EXCLUDE_FROM_ALL Examples)
WRITE_FILE(${SUBDIR_BINARY_DIR}/ShouldBeHere "This file should exist.")

ADD_EXECUTABLE(TestFromSubdir
  AnotherSubdir/testfromsubdir.c
  AnotherSubdir/secondone
  AnotherSubdir/pair+int.int.c
  vcl_algorithm+vcl_pair+double.foo.c
  )

AUX_SOURCE_DIRECTORY(ThirdSubDir SOURCES)
SET(SOURCES ${SOURCES} 
  vcl_algorithm+vcl_pair+double.foo.c)
MESSAGE("Sources: ${SOURCES}")
ADD_EXECUTABLE(TestWithAuxSourceDir ${SOURCES})
