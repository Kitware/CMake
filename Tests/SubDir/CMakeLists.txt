PROJECT(SUBDIR)
SUBDIRS(Executable EXCLUDE_FROM_ALL Examples)
WRITE_FILE(${SUBDIR_BINARY_DIR}/ShouldBeHere "This file should exist.")

ADD_EXECUTABLE(TestFromSubdir AnotherSubdir/testfromsubdir.c AnotherSubdir/secondone)
