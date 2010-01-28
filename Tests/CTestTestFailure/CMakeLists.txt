cmake_minimum_required (VERSION 2.6)
PROJECT(CTestTestFailure)
INCLUDE(CTest)

ADD_EXECUTABLE (NoBuild badCode.cxx)
TARGET_LINK_LIBRARIES (NoBuild ${EXTRA_LIBS})

ADD_TEST (TestNoExe NoBuild)
