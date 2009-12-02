cmake_minimum_required (VERSION 2.6)
PROJECT(CTestTestSubdir)
INCLUDE(CTest)

ADD_SUBDIRECTORY(subdir)
SUBDIRS(subdir2)
SUBDIRS("${CTestTestSubdir_SOURCE_DIR}/subdir3")
