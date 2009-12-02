cmake_minimum_required (VERSION 2.6)
PROJECT(CTestTestCrash)
INCLUDE(CTest)

ADD_EXECUTABLE (Crash crash.cxx)
TARGET_LINK_LIBRARIES (NoBuild ${EXTRA_LIBS})

ADD_TEST (TestCrash Crash)
