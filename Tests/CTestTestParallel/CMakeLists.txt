cmake_minimum_required (VERSION 2.6)
PROJECT(CTestTestParallel)
INCLUDE(CTest)

ADD_EXECUTABLE (LockFile lockFile.c)

ADD_TEST (TestRunSerial1 LockFile)
ADD_TEST (TestRunSerial2 LockFile)
SET_TESTS_PROPERTIES(TestRunSerial1 TestRunSerial2 PROPERTIES RUN_SERIAL true)

ADD_TEST (TestProcessorsGreaterThanMPL1 LockFile)
ADD_TEST (TestProcessorsGreaterThanMPL2 LockFile)
SET_TESTS_PROPERTIES(TestProcessorsGreaterThanMPL1 PROPERTIES PROCESSORS 10)
SET_TESTS_PROPERTIES(TestProcessorsGreaterThanMPL1 PROPERTIES DEPENDS
                     TestProcessorsGreaterThanMPL2)
